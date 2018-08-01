/**
 * @file   TDBSparseDescriptorSet.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
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
#include <tiledb/tiledb.h>

#define ATTRIBUTE_SPARSE_ID     "id"
#define ATTRIBUTE_SPARSE_LABEL "label"

#define DIMENSION_LOWER_LIMIT -10000
#define DIMENSION_UPPER_LIMIT 10000
#define DIMENSION_TILE_SIZE 250

using namespace VCL;

TDBSparseDescriptorSet::TDBSparseDescriptorSet(const std::string &filename):
    TDBDescriptorSet(filename)
{
    _name = "unnecessary_name";
    read_metadata();
}

TDBSparseDescriptorSet::TDBSparseDescriptorSet(
                    const std::string &filename,
                    uint32_t dim,
                    DistanceMetric metric):
    TDBDescriptorSet(filename, dim)
{
    _name = "unnecessary_name";
    tiledb::Domain domain(_tiledb_ctx);

    for (int i = 0; i < _dimensions; ++i) {
        std::string dim_name = "dim_" + std::to_string(i);

        float lower_limit = DIMENSION_LOWER_LIMIT;
        float upper_limit = DIMENSION_UPPER_LIMIT;

        if (i == 0) {
            // First dimension is incresed to store metadata,
            // as descriptors will always have at least 1 dim.
            upper_limit = upper_limit + DIMENSION_TILE_SIZE;
        }

        auto d = tiledb::Dimension::create<float>(
                        _tiledb_ctx, dim_name.c_str(), {
                        {lower_limit, upper_limit}}, DIMENSION_TILE_SIZE);
        // TODO: The domains can be set based on the distribution
        // of the data, which can be moved to trai() fuction in the future.
        domain.add_dimension(d);
    }

    tiledb::Attribute att_id = tiledb::Attribute::create<long>(_tiledb_ctx, ATTRIBUTE_SPARSE_ID);
    att_id.set_compressor({TILEDB_BLOSC_LZ, -1});

    tiledb::Attribute att_label = tiledb::Attribute::create<long>(_tiledb_ctx, ATTRIBUTE_SPARSE_LABEL);
    att_label.set_compressor({TILEDB_BLOSC_LZ, -1});

    tiledb::ArraySchema schema(_tiledb_ctx, TILEDB_SPARSE);
    schema.set_tile_order(TILEDB_ROW_MAJOR).set_cell_order(TILEDB_ROW_MAJOR);
    schema.set_capacity(100);
    schema.set_domain(domain);
    schema.add_attribute(att_id);
    schema.add_attribute(att_label);

    try {
        schema.check();
    } catch (tiledb::TileDBError &e) {
        throw VCLException(TileDBError, "Error setting TileDB Schema");
    }

    tiledb::Array::create(_set_path, schema);
}

void TDBSparseDescriptorSet::write_metadata()
{
    std::vector<float> coords(_dimensions, DIMENSION_UPPER_LIMIT);
    coords[0] += 1.0f;

    long dims = _dimensions;
    long n_total = _n_total;
    // We use the ID attribute to store _dimension
    // and the LABEL attibute to store _n_total;
    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_WRITE);
    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_GLOBAL_ORDER);
    query.set_buffer(ATTRIBUTE_SPARSE_ID, &dims, 1);
    query.set_buffer(ATTRIBUTE_SPARSE_LABEL, &n_total, 1);
    query.set_buffer(TILEDB_COORDS, coords);
    query.submit();
    query.finalize();
    array.close();
}

void TDBSparseDescriptorSet::read_metadata()
{
    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);

    std::vector<float> coords(_dimensions * 2, DIMENSION_UPPER_LIMIT);
    coords[0] += 1.0f;
    coords[1] += 1.0f;

    auto max_sizes = array.max_buffer_elements(coords);

    // Prepare cell buffers
    std::vector<long> desc_ids(max_sizes[ATTRIBUTE_SPARSE_ID].second);
    std::vector<long> desc_labels(max_sizes[ATTRIBUTE_SPARSE_LABEL].second);

    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR).set_subarray(coords);
    query.set_buffer(ATTRIBUTE_SPARSE_ID, desc_ids);
    query.set_buffer(ATTRIBUTE_SPARSE_LABEL, desc_labels);
    query.submit();

    auto result_el = query.result_buffer_elements();
    long found = result_el[ATTRIBUTE_SPARSE_ID].second;

    assert(found == 1);

    _dimensions = desc_ids[0];
    _n_total = desc_labels[0];

    array.close();
}

long TDBSparseDescriptorSet::add(float* descriptors, unsigned n, long* labels)
{
    try {
        std::vector<long> att_id(n);
        std::iota(att_id.begin(), att_id.end(), _n_total);

        std::vector<long> att_label;

        long* labels_for_query = labels;

        if (labels == NULL) {
            // By default, labels is -1
            att_label = std::vector<long> (n, -1);
            labels_for_query = att_label.data();
        }

        {
            tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_WRITE);
            tiledb::Query query(_tiledb_ctx, array);
            query.set_layout(TILEDB_GLOBAL_ORDER);
            query.set_buffer(ATTRIBUTE_SPARSE_ID, att_id);
            query.set_buffer(ATTRIBUTE_SPARSE_LABEL, labels_for_query, n);
            query.set_buffer(TILEDB_COORDS, descriptors, n * _dimensions);
            query.submit();
            query.finalize();
            array.close();
        }
    } catch (tiledb::TileDBError &e) {
        throw VCLException(UnsupportedOperation, "TileDBError, check logs");
    }

    write_metadata();
    _n_total += n;
    return _n_total - n;
}

void TDBSparseDescriptorSet::load_neighbors(float* q, unsigned k,
                                        std::vector<float>& descriptors,
                                        std::vector<long>& desc_ids,
                                        std::vector<long>& desc_labels)
{
    bool flag_found = true;
    long found = 0;
    int attempt = 0;

    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);

    while (found < k) {

        // Calculate maximum buffer elements for the
        // query results per attribute

        std::vector<float> subarray(_dimensions * 2);

        float space = std::pow(2, 4 + attempt++);

        if (space >= (DIMENSION_UPPER_LIMIT - DIMENSION_LOWER_LIMIT) / 2) {
            flag_found = false;
            break;
        }

        #pragma omp parallel for
        for (int i = 0; i < _dimensions; ++i) {
            subarray[2*i+0] = (q[i] - space) > DIMENSION_LOWER_LIMIT ?
                              (q[i] - space) : DIMENSION_LOWER_LIMIT;
            subarray[2*i+1] = (q[i] + space) < DIMENSION_UPPER_LIMIT ?
                              (q[i] + space) : DIMENSION_UPPER_LIMIT;
        }

        auto max_sizes = array.max_buffer_elements(subarray);

        // Prepare cell buffers
        descriptors.resize(max_sizes[TILEDB_COORDS].second);
        desc_ids.resize(max_sizes[ATTRIBUTE_SPARSE_ID].second);
        desc_labels.resize(max_sizes[ATTRIBUTE_SPARSE_LABEL].second);

        // Create query
        tiledb::Query query(_tiledb_ctx, array);
        query.set_layout(TILEDB_ROW_MAJOR).set_subarray(subarray);
        query.set_buffer(ATTRIBUTE_SPARSE_LABEL, desc_labels);
        query.set_buffer(ATTRIBUTE_SPARSE_ID, desc_ids);
        query.set_buffer(TILEDB_COORDS, descriptors);

        // Submit query
        query.submit();

        auto result_el = query.result_buffer_elements();
        found = result_el[ATTRIBUTE_SPARSE_ID].second;

        descriptors.resize(found * _dimensions);
        desc_ids.resize(found);
        desc_labels.resize(found);
    }

    array.close();

    if (flag_found == false) {
       desc_ids.clear();
       desc_labels.clear();
       descriptors.clear();
    }
}

void TDBSparseDescriptorSet::classify(float* descriptors, unsigned n,
                                         long* labels, unsigned quorum)
{
    float* distances  = new float[n * quorum];
    long*  ids_aux    = new long [n * quorum];
    long*  labels_aux = new long [n * quorum];

    search(descriptors, n, quorum, ids_aux, distances, labels_aux);

    for (int j = 0; j < n; ++j) {

        std::map<long, int> map_voting;
        long winner = -1;
        unsigned max = 0;
        for (int i = 0; i < quorum; ++i) {
            long idx = ids_aux[quorum*j + i];
            if (idx < 0)
                continue; // Means not found

            long label_id = labels_aux[quorum*j + i];
            map_voting[label_id] += 1;
            if (max < map_voting[label_id]) {
                max = map_voting[label_id];
                winner = label_id;
            }
        }
        labels[j] = winner;
    }
}

void TDBSparseDescriptorSet::search(float* query, unsigned n, unsigned k,
                                    long* ids, float* distances, long* labels)
{
    std::vector<float> descs;
    std::vector<long>  desc_ids;
    std::vector<long>  desc_labels;

    for (int i = 0; i < n; ++i) {

        load_neighbors(query + i * _dimensions, k, descs, desc_ids, desc_labels);
        unsigned found = desc_ids.size();

        std::vector<float> d(found);
        std::vector<long> idxs(found);

        compute_distances(query + i * _dimensions, d, descs);

        std::iota(idxs.begin(), idxs.end(), 0);
        std::partial_sort(idxs.begin(), idxs.begin() + k, idxs.end(),
                [&d](size_t i1, size_t i2) { return d[i1] < d[i2]; });

        for (int j = 0; j < std::min(k, found); ++j) {
            ids      [i * k + j] = desc_ids[idxs[j]];
            distances[i * k + j] = d[idxs[j]];
        }

        if ( k > found) {
            for (int j = found; j < k; ++j) {
                ids      [i * k + j] = -1;
                distances[i * k + j] = -1;
            }
        }

        // Include labels, needed for faster classify
        // because it already gets the labels from the load_neighbor()
        if (labels != NULL) {
            for (int j = 0; j < std::min(k, found); ++j) {
                labels   [i * k + j] = desc_labels[idxs[j]];
            }

            if ( k > found) {
                for (int j = found; j < k; ++j) {
                    labels   [i * k + j] = -1;
                }
            }
        }
    }
}

void TDBSparseDescriptorSet::search(float* query, unsigned n, unsigned k,
                                       long* ids, float* distances)
{
    search(query, n, k, ids, distances, NULL);
}

void TDBSparseDescriptorSet::get_descriptors(long* ids, unsigned n,
                                                float* descriptors)
{
    std::vector<float> subarray(_dimensions * 2);

    float space = 20;

    #pragma omp parallel for
    for (int i = 0; i < _dimensions; ++i) {
        subarray[2*i+0] = DIMENSION_LOWER_LIMIT;
        subarray[2*i+1] = DIMENSION_UPPER_LIMIT;
    }

    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);
    auto max_sizes = array.max_buffer_elements(subarray);

    // Prepare cell buffers
    std::vector<float> buffer;
    buffer.resize(max_sizes[TILEDB_COORDS].second);
    std::vector<long>  desc_ids;
    desc_ids.resize(max_sizes[ATTRIBUTE_SPARSE_ID].second);

    // Create query
    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR);
    query.set_subarray(subarray);
    query.set_buffer(ATTRIBUTE_SPARSE_ID, desc_ids);
    query.set_buffer(TILEDB_COORDS, buffer);

    // Submit query
    query.submit();

    // Print cell values (assumes all attributes are read)
    auto result_el = query.result_buffer_elements();
    unsigned n_found = result_el[ATTRIBUTE_SPARSE_ID].second;

    buffer.resize(n_found * _dimensions);
    desc_ids.resize(n_found);

    // This is the worst algorithm ever, EVER.
    // This is O(n), can be implemented using a binary search.
    // We need to sort the desc_ids for this, need a trade off.

    for (int i = 0; i < n; ++i) {
        long offset = i *_dimensions;
        long id_q = ids[i];
        bool found = false;

        for (int j = 0; j < desc_ids.size(); ++j) {
            if (id_q == desc_ids[j]) {
                std::memcpy(descriptors + offset,
                            &buffer[j * _dimensions],
                            sizeof(float) * _dimensions);
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        for (int j = 0; j < _dimensions; ++j) {
            descriptors[offset+j] = -1;
        }
    }
}

void TDBSparseDescriptorSet::get_labels(long* ids, unsigned n, long* labels)
{
    std::vector<float> subarray(_dimensions * 2);

    #pragma omp parallel for
    for (int i = 0; i < _dimensions; ++i) {
        subarray[2*i+0] = DIMENSION_LOWER_LIMIT;
        subarray[2*i+1] = DIMENSION_UPPER_LIMIT;
    }

    tiledb::Array array(_tiledb_ctx, _set_path, TILEDB_READ);
    auto max_sizes = array.max_buffer_elements(subarray);

    // Prepare cell buffers
    std::vector<long>  desc_ids;
    desc_ids.resize(max_sizes[ATTRIBUTE_SPARSE_ID].second);
    std::vector<long>  desc_labels;
    desc_labels.resize(max_sizes[ATTRIBUTE_SPARSE_LABEL].second);

    // Create query
    tiledb::Query query(_tiledb_ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR).set_subarray(subarray);
    query.set_buffer(ATTRIBUTE_SPARSE_LABEL, desc_labels);
    query.set_buffer(ATTRIBUTE_SPARSE_ID, desc_ids);

    // Submit query
    query.submit();

    // Print cell values (assumes all attributes are read)
    auto result_el = query.result_buffer_elements();
    unsigned n_found = result_el[ATTRIBUTE_SPARSE_ID].second;

    desc_ids.resize(n_found);
    desc_labels.resize(n_found);

    // This is the worst algo ever, EVER.
    // This is O(n), can be implemented using a binary search.
    // We need to sort the desc_ids for this, need a trade off.

    for (int i = 0; i < n; ++i) {
        long offset = i *_dimensions;
        long id_q = ids[i];
        bool found = false;

        for (int j = 0; j < desc_ids.size(); ++j) {
            if (id_q == desc_ids[j]) {
                labels[i] = desc_labels[j];
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        labels[i] = -1;
    }
}
