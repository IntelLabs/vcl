/**
 * @file   TDBDenseDescriptorSet.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files
 * (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

#include "TDBDescriptorSet.h"

#include <tiledb/map.h>

#define ATTRIBUTE_DESC  "descriptor"
#define ATTRIBUTE_LABEL "label"

using namespace VCL;

TDBDenseDescriptorSet::TDBDenseDescriptorSet(const std::string &filename):
    TDBDescriptorSet(filename),
    _flag_buffer_updated(false)
{
    read_metadata();
}

TDBDenseDescriptorSet::TDBDenseDescriptorSet(const std::string &filename,
                                     uint32_t dim,
                                     DistanceMetric metric):
    TDBDescriptorSet(filename, dim),
    _flag_buffer_updated(true)
{
    auto d = tiledb::Dimension::create<uint64_t>(
                            _tiledb_ctx, "d", {{0, MAX_DESC-1}}, 10);

    tiledb::Domain domain(_tiledb_ctx);
    domain.add_dimension(d);

    tiledb::Attribute a_desc = tiledb::Attribute::create<float>(
                                _tiledb_ctx, ATTRIBUTE_DESC);
    a_desc.set_compressor({TILEDB_BLOSC_LZ, -1});
    a_desc.set_cell_val_num(_dimensions);

    tiledb::Attribute a_label = tiledb::Attribute::create<long>(
                _tiledb_ctx, ATTRIBUTE_LABEL);
    a_label.set_compressor({TILEDB_BLOSC_LZ, -1});

    tiledb::ArraySchema schema(_tiledb_ctx, TILEDB_DENSE);
    schema.set_tile_order(TILEDB_ROW_MAJOR).set_cell_order(TILEDB_ROW_MAJOR);
    schema.set_domain(domain);
    schema.add_attribute(a_desc);
    schema.add_attribute(a_label);

    try {
        schema.check();
    } catch (tiledb::TileDBError &e) {
        throw VCLException(TileDBError, "Error creating TDB schema");
    }

    // Create array
    tiledb::Array::create(_set_path, schema);

    write_metadata();
}

void TDBDenseDescriptorSet::read_metadata()
{
    std::vector<long> metadata(2);

    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);
    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR);
    query.set_subarray<uint64_t>( {METADATA_OFFSET, METADATA_OFFSET+1} );
    query.set_buffer(ATTRIBUTE_LABEL, metadata);
    query.submit();

    _dimensions = (unsigned)metadata[0];
    _n_total    = (uint64_t)metadata[1];

}

void TDBDenseDescriptorSet::write_metadata()
{
    // Writing metadata using KV store in TileDB
    // (thru the write_metadata(3) in TDBObject)
    // is an overkill.
    // We simply need to store 3-64bits values as
    // metadata, which can be inserted in the
    // 2-D array that we are already using.
    // The following code does that, and the corresponding
    // read_metadata() knows how to read these values back.

    std::vector<long> metadata;
    metadata.push_back(_dimensions);
    metadata.push_back(_n_total);

    // This is only here because tiledb requires all the
    // attributes when writing.
    std::vector<float> aux_dims(_dimensions * 2, .0f);

    // Write metadata
    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_WRITE);
    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR);
    query.set_subarray<uint64_t>({METADATA_OFFSET, METADATA_OFFSET+1});
    query.set_buffer(ATTRIBUTE_LABEL, metadata);
    query.set_buffer(ATTRIBUTE_DESC, aux_dims);
    query.submit();
    query.finalize();

}

void TDBDenseDescriptorSet::load_buffer()
{
    try {

        read_metadata();

        tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);
        {
            _buffer.resize(_dimensions * _n_total);
            _label_ids.resize(_n_total);

            tiledb::Query query(_tiledb_ctx, array);
            query.set_layout(TILEDB_ROW_MAJOR);
            query.set_subarray<uint64_t>({0, _n_total - 1});
            query.set_buffer(ATTRIBUTE_DESC, _buffer);
            query.set_buffer(ATTRIBUTE_LABEL, _label_ids);
            query.submit();
        }

    } catch (tiledb::TileDBError &e) {
        throw VCLException(TileDBError, "Error: Reading Dense array");
    }

    _flag_buffer_updated = true;
}

long TDBDenseDescriptorSet::add(float* descriptors, unsigned n, long* labels)
{
    try {
        std::vector<long> att_label;
        long* labels_buffer = labels;

        if (labels == NULL) {
            // By default, labels is -1
            att_label = std::vector<long> (n, -1);
            labels_buffer = att_label.data();
        }

        {
            tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_WRITE);
            tiledb::Query query(_tiledb_ctx, array);
            query.set_layout(TILEDB_ROW_MAJOR);
            query.set_subarray<uint64_t>({_n_total, _n_total + n-1});
            query.set_buffer(ATTRIBUTE_DESC, descriptors, n * _dimensions);
            query.set_buffer(ATTRIBUTE_LABEL, labels_buffer, n);
            query.submit();
            query.finalize();
        }
    } catch (tiledb::TileDBError &e) {
        _flag_buffer_updated = false;
        throw VCLException(UnsupportedOperation, e.what());
    }

    // Write _n_total into tiledb
    // This is good because we only write metadata
    // (_n_total) after the other two writes succedded.
    _n_total += n;
    write_metadata();

    // - n becase we already increase _n_total for writing metadata on tdb
    long old_n_total = _n_total - n;

    _buffer.resize((_n_total) * _dimensions);
    std::memcpy(&_buffer[old_n_total * _dimensions], descriptors,
                n * _dimensions * sizeof(float));

    if (labels != NULL) {
        _label_ids.resize(_n_total);
        std::memcpy(&_label_ids[old_n_total], labels, n * sizeof(long));
    }

    return old_n_total;
}

void TDBDenseDescriptorSet::search(float* query,
                                      unsigned n_queries, unsigned k,
                                      long* ids, float* distances)
{
    if (!_flag_buffer_updated) {
        load_buffer();
    }

    std::vector<float> d(_n_total);
    std::vector<long> idxs(_n_total);

    for (int i = 0; i < n_queries; ++i) {

        compute_distances(query + i * _dimensions, d, _buffer);
        std::iota(idxs.begin(), idxs.end(), 0);
        std::partial_sort(idxs.begin(), idxs.begin() + k, idxs.end(),
                [&d](size_t i1, size_t i2) { return d[i1] < d[i2]; });

        for (int j = 0; j < k; ++j) {
            ids      [i * k + j] = idxs[j];
            distances[i * k + j] = d[idxs[j]];
        }
    }
}

void TDBDenseDescriptorSet::get_descriptors(long* ids, unsigned n,
                                              float* descriptors)
{
    if (!_flag_buffer_updated) {
        load_buffer();
    }

    for (int i = 0; i < n; ++i) {
        long idx = ids[i] * _dimensions;
        long offset = i *_dimensions;
        std::memcpy(descriptors + offset, &_buffer[idx],
                    sizeof(float) * _dimensions);
    }
}
