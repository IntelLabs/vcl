/**
 * @file   TDBObject.cc
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

#include <stddef.h>
#include <string>

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <tiledb.h>
#include "TDBObject.h"
#include "Exception.h"

using namespace VCL;

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */

TDBObject::TDBObject()
{
    Error_Check(
        tiledb_ctx_init(&_ctx, NULL),
        "TileDB context initialization failed");

    _workspace = "";
    _group = "";
    _name = "";

    // set default values
    _num_attributes = 1;
    _attributes.push_back("value");
    _compressed = CompressionType::LZ4;
    _min_tile_dimension = 4;
}

TDBObject::TDBObject(const std::string &image_id)
{
    Error_Check(
        tiledb_ctx_init(&_ctx, NULL),
        "TileDB context initialization failed");

    size_t pos = get_path_delimiter(image_id);

    _workspace = get_workspace(image_id, pos);
    _group = get_group(image_id, pos);
    _name = get_name(image_id, pos);

    // set default values
    _num_attributes = 1;
    _attributes.push_back("value");
    _compressed = CompressionType::LZ4;
    _min_tile_dimension = 4;
}

TDBObject::TDBObject(const TDBObject &tdb)
{
    Error_Check(
        tiledb_ctx_init(&_ctx, NULL),
        "TileDB context initialization failed");

    set_equal(tdb);
}


TDBObject& TDBObject::operator=(const TDBObject &tdb)
{
    Error_Check(
        tiledb_ctx_finalize(_ctx),
        "TileDB context finalization failed");
    Error_Check(
        tiledb_ctx_init(&_ctx, NULL),
        "TileDB context initialization failed");

    reset_arrays();

    set_equal(tdb);

    return *this;
}

void TDBObject::set_equal(const TDBObject &tdb)
{
    _workspace = tdb._workspace;
    _group = tdb._group;

    _num_attributes = tdb._num_attributes;
    _attributes.clear();
    for ( int i = 0; i < tdb._attributes.size(); ++i )
        _attributes.push_back(tdb._attributes[i]);

    _num_dimensions = tdb._num_dimensions;
    _dimension_names.clear();
    _dimension_values.clear();
    for ( int i = 0; i < tdb._dimension_names.size(); ++i )
        _dimension_names.push_back(tdb._dimension_names[i]);
    for ( int i = 0; i < tdb._dimension_values.size(); ++i )
        _dimension_values.push_back(tdb._dimension_values[i]);

    _compressed = tdb._compressed;
    _min_tile_dimension = tdb._min_tile_dimension;
    _array_dimension = tdb._array_dimension;
    _tile_dimension = tdb._tile_dimension;
}

TDBObject::~TDBObject()
{
    reset_arrays();
    Error_Check(
        tiledb_ctx_finalize(_ctx),
        "TileDB context finalization failed");
}

void TDBObject::reset_arrays()
{
    _attributes.clear();
    _attributes.shrink_to_fit();
    _dimension_names.clear();
    _dimension_names.shrink_to_fit();
    _dimension_values.clear();
    _dimension_values.shrink_to_fit();
}

void TDBObject::delete_object()
{
    std::string object_id = _group + _name;
    Error_Check(
        tiledb_delete(_ctx, object_id.c_str()),
        "TileDB delete failed");
}

    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string TDBObject::get_image_id() const
{
    return _group + _name;
}


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */

void TDBObject::set_num_dimensions(int num)
{
    _num_dimensions = num;
}

void TDBObject::set_dimensions(std::vector<std::string> dimensions)
{
    _dimension_names.clear();

    for ( int x = 0; x < dimensions.size(); ++x ){
        _dimension_names.push_back(dimensions[x]);
    }
}

void TDBObject::set_dimension_values(std::vector<int> dimensions)
{
    _dimension_values.clear();

    for ( int x = 0; x < dimensions.size(); ++x ){
        _dimension_values.push_back(dimensions[x]);
    }
}

void TDBObject::set_minimum(int dimension)
{
    std::cout << dimension << std::endl;
    _min_tile_dimension = dimension;
}

void TDBObject::set_num_attributes(int num)
{
    _num_attributes = num;
    set_default_attributes();
}

void TDBObject::set_attributes(std::vector<std::string> attributes)
{
    _attributes.clear();

    for ( int x = 0; x < attributes.size(); ++x ){
        _attributes.push_back(attributes[x]);
    }
}

void TDBObject::set_compression(CompressionType comp)
{
    _compressed = comp;
}




    /*  *********************** */
    /*  PROTECTED GET FUNCTIONS */
    /*  *********************** */

size_t TDBObject::get_path_delimiter( const std::string &filename ) const
{
    std::string delimiter = "/";

    size_t pos = filename.rfind(delimiter);
    if ( pos == filename.length() - 1 ) {
        std::string file = filename.substr(0, pos);
        pos = file.rfind(delimiter);
    }

    return pos;
}

std::string TDBObject::get_workspace(const std::string &filename, size_t pos) const
{
    std::string substring = filename.substr(0, pos);
    size_t loc = substring.rfind("/");

    std::string workspace = substring.substr(0, loc + 1);

    if ( tiledb_dir_type(_ctx, workspace.c_str()) != TILEDB_WORKSPACE ) {
        Error_Check(
            tiledb_workspace_create(_ctx, workspace.c_str()),
            "Cannot create the TileDB workspace " + workspace + "; directory may \
            already exist or parent is another TileDB directory");
    }

    return workspace;
}

std::string TDBObject::get_group(const std::string &filename, size_t pos) const
{
    std::string group = filename.substr(0, pos + 1);

    if ( tiledb_dir_type(_ctx, group.c_str()) != TILEDB_GROUP ) {
        Error_Check(
            tiledb_group_create(_ctx, group.c_str()),
            "Cannot create the TileDB group " + group + "; directory may \
            already exist or parent directory is not a TileDB directory");
    }
    return group;
}

std::string TDBObject::get_name(const std::string &filename, size_t pos) const
{
    std::string id = filename.substr(pos + 1);
    return id;
}



    /*  *********************** */
    /*  PROTECTED SET FUNCTIONS */
    /*  *********************** */

void TDBObject::set_schema(int cell_val_num_, const std::string &object_file)
{
    TileDB_ArraySchema array_schema;

    const char* array_name = object_file.c_str();
    const char* attributes[_num_attributes];
    int cell_val_num[_num_attributes];

    for (int x = 0; x < _attributes.size(); ++x) {
        attributes[x] = _attributes[x].c_str();
    }

    for ( int i = 0; i < _num_attributes; ++i ) {
        cell_val_num[i] = cell_val_num_;
    }

    int cell_order = TILEDB_ROW_MAJOR;

    int compression[_num_attributes + 1];
    for ( int i = 0; i < _num_attributes; ++i ){
        compression[i] = int(_compressed);
    }
    compression[_num_attributes] = TILEDB_NO_COMPRESSION;

    // determine array dimensions and tile sizes
    find_tile_extents();

    int dense = 1;
    const char* dimensions[_num_dimensions];
    uint64_t domain[_num_dimensions*2];

    for (int x = 0; x < _dimension_names.size(); ++x) {
        dimensions[x] = _dimension_names[x].c_str();
    }

    int y = 0;
    for (int x = 0; x < _num_dimensions*2; x+=2) {
        domain[x] = 0;
        domain[x+1] = _array_dimension[y] - 1;
        ++y;
    }

    size_t domain_len = (_num_dimensions * 2) * sizeof(uint64_t);

    uint64_t tile_extents[_num_dimensions];
    int64_t capacity = 0;
    for (int x = 0; x < _num_dimensions; ++x) {
        tile_extents[x] = uint64_t(_tile_dimension[x]);
        if ( capacity == 0 )
            capacity += _tile_dimension[x];
        else
            capacity *= _tile_dimension[x];
    }

    size_t tile_extents_len = 2*sizeof(uint64_t);
    int tile_order = TILEDB_ROW_MAJOR;

    int types[_num_attributes + 1];
    set_types(types);

    types[_num_attributes] = TILEDB_INT64;

    Error_Check(
        tiledb_array_set_schema(&array_schema, array_name, attributes,
            _num_attributes, capacity, cell_order, cell_val_num,
            compression, dense, dimensions, _num_dimensions, domain, domain_len,
            tile_extents, tile_extents_len, tile_order, types),
        "TileDB schema setup failed");

    if ( tiledb_dir_type(_ctx, object_file.c_str()) != TILEDB_ARRAY ){
        Error_Check(
            tiledb_array_create(_ctx, &array_schema),
            "Cannot create the TileDB directory " + object_file + "; \
            directory may already exist or parent directory is not a TileDB directory");
        }

    Error_Check(
        tiledb_array_free_schema(&array_schema),
        "TileDB schema failed to be freed");
}


void TDBObject::set_from_schema(TileDB_Array* tiledb_array)
{
    TileDB_ArraySchema schema;
    Error_Check(
        tiledb_array_get_schema(tiledb_array, &schema),
        "TileDB schema retrieval failed");

    _num_attributes = schema.attribute_num_;
    _num_dimensions = schema.dim_num_;

    int64_t* tiles = (int64_t*) schema.tile_extents_;
    int64_t* domain = (int64_t*) schema.domain_;

    for (int i = 0; i < _num_dimensions; ++i)
        _tile_dimension.push_back(tiles[i]);
    for (int i = 0; i < _num_dimensions*2; i+=2)
        _array_dimension.push_back(domain[i + 1] + 1);

    Error_Check(
        tiledb_array_free_schema(&schema),
        "TileDB schema failed to be freed");
}


    /*  *********************** */
    /*   METADATA INTERACTION   */
    /*  *********************** */

void TDBObject::write_metadata(const std::string &metadata, int64_t *buffer,
    char* buffer_var_keys, size_t* buffer_keys, size_t var_keys_size)
{
    const char* metadata_name = metadata.c_str();
    const char* attributes[] = { "dimensions" };
    const int capacity = 4;
    const int cell_val_num[] = { 1 };

    const int compression[] = {int(_compressed), TILEDB_NO_COMPRESSION};
    const int types[] = { TILEDB_INT64 };

    TileDB_MetadataSchema metadata_schema;
    Error_Check(
        tiledb_metadata_set_schema(&metadata_schema, metadata_name,
            attributes, 1, capacity, cell_val_num, compression, types),
        "TileDB metadata schema setup failed");
    if ( tiledb_dir_type(_ctx, metadata_name) != TILEDB_METADATA ) {
        Error_Check(
            tiledb_metadata_create(_ctx, &metadata_schema),
            "Cannot create the TileDB directory " + metadata + "; directory may \
            already exist or parent directory is not a TileDB directory");
    }
    Error_Check(
        tiledb_metadata_free_schema(&metadata_schema),
        "TileDB schema failed to be freed");

    TileDB_Metadata* tiledb_metadata;
    Error_Check(
        tiledb_metadata_init(_ctx, &tiledb_metadata, metadata_name,
            TILEDB_METADATA_WRITE, NULL, 0),
        "TileDB metadata initialization failed");

    int size = _num_dimensions + 1;

    const void* buffers[] = { buffer, buffer_keys, buffer_var_keys };
    size_t buffer_sizes[] = { sizeof(buffer) * size,
        sizeof(buffer_keys) * size, var_keys_size };

    Error_Check(
        tiledb_metadata_write(tiledb_metadata, buffer_var_keys,
            var_keys_size, buffers, buffer_sizes),
        "TileDB metadata failed to write");

    Error_Check(
        tiledb_metadata_finalize(tiledb_metadata),
        "TileDB metadata failed to finalize");
}

void TDBObject::find_tile_extents()
{
    _array_dimension.clear();
    _tile_dimension.clear();
    for (int x = 0; x < _num_dimensions; ++x) {
        int dimension = _dimension_values[x];
        int num_tiles = 0;

        int gf_dimension = greatest_factor(dimension);
        
        while ( gf_dimension == 1 ) {
            dimension = dimension + 1;
            gf_dimension = greatest_factor(dimension);
        }

        _array_dimension.push_back(dimension);
        _tile_dimension.push_back(gf_dimension);
    }
}


    /*  *********************** */
    /*   PRIVATE FUNCTIONS      */
    /*  *********************** */
void TDBObject::set_types(int* types)
{
    for ( int i = 0; i < _num_attributes; ++i ){
        types[i] = TILEDB_CHAR;
    }
}

int TDBObject::greatest_factor(int a)
{
    int b = a;

    while (b > 1) {
//        --b;
       if (a % b == 0 && a / b >= _min_tile_dimension)
       {
           return b;
       }
       --b;
    }
    return b;
}



