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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#include "TDBObject.h"
#include "Exception.h"

using namespace VCL;

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */

TDBObject::TDBObject()
{
    Error_Check(
        tiledb_ctx_create(&_ctx, NULL), 
        _ctx, "TileDB context initialization failed");

    _group = "";
    _name = "";

    // set default values
    _num_attributes = 1;
    const char* attr = "value";
    _attributes.push_back(attr);
    _compressed = CompressionType::LZ4;
    _min_tile_dimension = 4;

    _config = NULL;
    _error = NULL;
    _vfs = NULL;
}

TDBObject::TDBObject(const std::string &image_id)
{
    Error_Check(
        tiledb_ctx_create(&_ctx, NULL), 
        _ctx, "TileDB context initialization failed");

    size_t pos = get_path_delimiter(image_id);

    _group = get_group(image_id, pos);
    _name = get_name(image_id, pos);

    // set default values
    _num_attributes = 1;
    const char* attr = "value";
    _attributes.push_back(attr);
    _compressed = CompressionType::LZ4;
    _min_tile_dimension = 4;

    _config = NULL;
    _error = NULL;
    _vfs = NULL;
}

TDBObject::TDBObject(const std::string &image_id, RemoteConnection &connection)
{
    Error_Check(
        tiledb_ctx_create(&_ctx, NULL), 
        _ctx, "TileDB context initialization failed");

    set_config(connection);

    size_t pos = get_path_delimiter(image_id);

    _group = get_group(image_id, pos);
    _name = get_name(image_id, pos);

    // set default values
    _num_attributes = 1;
    const char* attr = "value";
    _attributes.push_back(attr);
    _compressed = CompressionType::LZ4;
    _min_tile_dimension = 4;
}

TDBObject::TDBObject(const TDBObject &tdb)
{
    Error_Check(
        tiledb_ctx_create(&_ctx, tdb._config), 
        _ctx, "TileDB context initialization failed");

    set_equal(tdb);
}


TDBObject& TDBObject::operator=(const TDBObject &tdb)
{
    Error_Check(
        tiledb_ctx_free(&_ctx), _ctx,
        "TileDB context finalization failed");
    Error_Check(
        tiledb_ctx_create(&_ctx, tdb._config), _ctx,
        "TileDB context initialization failed");

    reset_arrays();

    set_equal(tdb);

    return *this;
}

void TDBObject::set_equal(const TDBObject &tdb)
{
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

    _config = tdb._config;
    _error = tdb._error;
    _vfs = tdb._vfs;
}

TDBObject::~TDBObject()
{
    reset_arrays();

    if ( _error != NULL )
        Error_Check(
            tiledb_error_free(&_error), 
            _ctx, "TileDB failed to free error\n");

    if ( _vfs != NULL )    
        Error_Check(
            tiledb_vfs_free(_ctx, &_vfs), 
            _ctx, "TileDB failed to free VFS\n");

    Error_Check(
        tiledb_ctx_free(&_ctx), 
        _ctx, "TileDB context finalization failed\n");
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
    std::string md_id = _group + _name + "/metadata";
    tiledb_object_t type;
    tiledb_object_type(_ctx, md_id.c_str(), &type);
    if (type == TILEDB_KEY_VALUE) {
        Error_Check(
            tiledb_object_remove(_ctx, md_id.c_str()),
            _ctx,
            "TileDB failed to delete metadata for " + object_id);
    }
    Error_Check(
        tiledb_object_remove(_ctx, object_id.c_str()),
        _ctx,
        "TileDB failed to delete " + object_id);
}

    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string TDBObject::get_object_id() const
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

void TDBObject::set_dimensions(const std::vector<std::string> &dimensions)
{
    _dimension_names.clear();

    for ( int x = 0; x < dimensions.size(); ++x ){
        _dimension_names.push_back(dimensions[x]);
    }
}

void TDBObject::set_dimension_values(const std::vector<uint64_t> &dimensions)
{
    _dimension_values.clear();

    for ( int x = 0; x < dimensions.size(); ++x ){
        _dimension_values.push_back(dimensions[x]);
    }
}

void TDBObject::set_minimum(int dimension)
{
    _min_tile_dimension = dimension;
}

void TDBObject::set_num_attributes(int num)
{
    _num_attributes = num;
    set_default_attributes();
}

void TDBObject::set_attributes(const std::vector<std::string> &attributes)
{
    _attributes.clear();

    for ( int x = 0; x < attributes.size(); ++x ){
        _attributes.push_back(const_cast<char*>(attributes[x].c_str()));
    }
}

void TDBObject::set_compression(CompressionType comp)
{
    _compressed = comp;
}

void TDBObject::set_config(RemoteConnection &remote)
{
    _error = NULL;
    Error_Check(
        tiledb_config_create(&_config, &_error),
        _ctx, "TileDB config creation failed\n");

    #ifdef S3_SUPPORT
        std::string region = remote.get_s3_region();
        std::string connect_timeout = std::to_string(remote.get_s3_connect_timeout());
        std::string result_timeout = std::to_string(remote.get_s3_result_timeout());

        if ( setenv("AWS_ACCESS_KEY_ID", remote.get_s3_access_id().c_str(), 1) < 0 )
            throw VCLException(IncorrectConfiguration, "Failed to set environment variable for TileDB");
        if ( setenv("AWS_SECRET_ACCESS_KEY", remote.get_s3_secret_key().c_str(), 1) < 0 )
            throw VCLException(IncorrectConfiguration, "Failed to set environment variable for TileDB");

        Error_Check(
            tiledb_config_set(_config, "vfs.s3.region", region.c_str(), &_error),
            _ctx, "Setting the S3 region in the TileDB config failed\n");
        Error_Check(
            tiledb_config_set(_config, "vfs.s3.connect_timeout_ms", connect_timeout.c_str(), &_error),
            _ctx, "Setting the S3 connection timeout in the TileDB config failed\n");
        Error_Check(
            tiledb_config_set(_config, "vfs.s3.result_timeout_ms", result_timeout.c_str(), &_error),
            _ctx, "Setting the S3 result timeout in the TileDB config failed\n");
    #endif

    Error_Check(
        tiledb_ctx_free(&_ctx), 
        _ctx, "TileDB context finalization failed");

    Error_Check(
        tiledb_ctx_create(&_ctx, _config),
        _ctx, "TileDB ctx creation failed\n");
    Error_Check(
        tiledb_vfs_create(_ctx, &_vfs, _config), 
        _ctx, "TileDB VFS creation failed\n");
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

std::string TDBObject::get_group(const std::string &filename, size_t pos) const
{
    std::string group = filename.substr(0, pos + 1);

    tiledb_object_t type;
    Error_Check(
        tiledb_object_type(_ctx, group.c_str(), &type), 
        _ctx,
        "Type check of " + group + " failed in TileDB");

    if ( type != TILEDB_GROUP ) {
        Error_Check(
            tiledb_group_create(_ctx, group.c_str()),
            _ctx,
            "Cannot create the TileDB group " + group +
            "; directory may already exist or parent directory is not a TileDB directory");
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
void TDBObject::set_schema_attributes(tiledb_array_schema_t* array_schema, int cell_val_num)
{
    for (int x = 0; x < _attributes.size(); ++x) {
        tiledb_attribute_t* attr;
        Error_Check(
            tiledb_attribute_create(_ctx, &attr, _attributes[x], TILEDB_CHAR), 
            _ctx,
            "TileDB failed to create attribute");
        tiledb_compressor_t compressor = convert_to_tiledb();
        Error_Check(
            tiledb_attribute_set_compressor(_ctx, attr, compressor, -1), 
            _ctx,
            "TileDB failed to set attribute information");
        Error_Check(
            tiledb_attribute_set_cell_val_num(_ctx, attr, cell_val_num),
            _ctx,
            "TileDB failed to set attribute information");

        Error_Check(
            tiledb_array_schema_add_attribute(_ctx, array_schema, attr),
            _ctx,
            "TileDB failed to add attribute to schema");
        Error_Check(
            tiledb_attribute_free(_ctx, &attr), _ctx,
            "TileDB failed to free attribute object");
    }
}

void TDBObject::set_schema_dimensions(tiledb_array_schema_t* array_schema)
{
    tiledb_domain_t* domain;
    Error_Check(
        tiledb_domain_create(_ctx, &domain), _ctx,
        "TileDB failed to create domain object");

    find_tile_extents();

    uint64_t domains[_num_dimensions][2];
    int y = 0;
    for (int x = 0; x < _num_dimensions; ++x) {
        domains[x][0] = 0;
        domains[x][1] = _array_dimension[y] - 1;
        ++y;
    }

    for (int x = 0; x < _num_dimensions; ++x) {
        uint64_t cur_domain[] = {domains[x][0], domains[x][1]};
        tiledb_dimension_t* dim;

        Error_Check(
            tiledb_dimension_create(_ctx, &dim, _dimension_names[x].c_str(), 
                TILEDB_UINT64, cur_domain, &_tile_dimension[x]),
            _ctx,
            "TileDB failed to create dimension " + _dimension_names[x]);

        Error_Check(
            tiledb_domain_add_dimension(_ctx, domain, dim), _ctx,
            "TileDB failed to add dimension " + _dimension_names[x]);
        Error_Check(
            tiledb_dimension_free(_ctx, &dim), _ctx,
            "TileDB failed to create dimension " + _dimension_names[x]);
    }

    Error_Check(
        tiledb_array_schema_set_domain(_ctx, array_schema, domain),
        _ctx,
        "TileDB failed to set domain");
    Error_Check(
        tiledb_domain_free(_ctx, &domain), _ctx,
        "TileDB failed to free domain object");
}

void TDBObject::set_schema_dense(int cell_val_num, const std::string& object_id)
{
    tiledb_array_schema_t* array_schema;
    Error_Check(
        tiledb_array_schema_create(_ctx, &array_schema, TILEDB_DENSE),
        _ctx,
        "TileDB failed to create array schema for " + object_id);
    set_schema(cell_val_num, object_id, array_schema);
}

void TDBObject::set_schema_sparse(int cell_val_num, const std::string& object_id)
{
    tiledb_array_schema_t* array_schema;
    Error_Check(
        tiledb_array_schema_create(_ctx, &array_schema, TILEDB_SPARSE),
        _ctx,
        "TileDB failed to create array schema for " + object_id);
    set_schema(cell_val_num, object_id, array_schema);
}

void TDBObject::set_schema(int cell_val_num, const std::string& object_id, tiledb_array_schema_t* array_schema)
{
    Error_Check(
        tiledb_array_schema_set_cell_order(_ctx, array_schema, TILEDB_ROW_MAJOR),
        _ctx,
        "TileDB failed to set array schema information for " + object_id);
    Error_Check(
        tiledb_array_schema_set_tile_order(_ctx, array_schema, TILEDB_ROW_MAJOR),
        _ctx,
        "TileDB failed to set array schema information for " + object_id);

    set_schema_attributes(array_schema, cell_val_num);
    set_schema_dimensions(array_schema);

    Error_Check(
        tiledb_array_schema_check(_ctx, array_schema),
        _ctx,
        "TileDB schema setup failed");

    tiledb_object_t type;
    Error_Check(
        tiledb_object_type(_ctx, object_id.c_str(), &type),
        _ctx,
        "TileDB object check of " + object_id + " failed");
    if ( type != TILEDB_ARRAY ){
        Error_Check(
            tiledb_array_create(_ctx, object_id.c_str(), array_schema),
            _ctx,
            "Cannot create the TileDB array " + object_id + 
            "; array may already exist or parent directory is not a TileDB directory");
        }

    Error_Check(
        tiledb_array_schema_free(_ctx, &array_schema),
        _ctx,
        "TileDB schema failed to be freed");
}


void TDBObject::set_from_schema(const std::string &object_id)
{
    tiledb_array_schema_t* array_schema;
    Error_Check(
        tiledb_array_schema_load(_ctx, &array_schema, object_id.c_str()),
        _ctx,
        "TileDB schema retrieval failed");

    unsigned attr_num;
    Error_Check(
        tiledb_array_schema_get_attribute_num(_ctx, array_schema, &attr_num),
        _ctx,
        "TileDB failed to retrieve attribute from schema");
    _num_attributes = attr_num;

    tiledb_domain_t* domain;
    Error_Check(
        tiledb_array_schema_get_domain(_ctx, array_schema, &domain),
        _ctx,
        "TileDB failed to retrieve domain from schema");
    unsigned dim_num;
    Error_Check(
        tiledb_domain_get_rank(_ctx, domain, &dim_num),
        _ctx,
        "TileDB failed to get dimension number from domain");
    _num_dimensions = dim_num;

    for (int i = 0; i < _num_dimensions; ++i) {
        tiledb_dimension_t* dim;
        Error_Check(
            tiledb_domain_get_dimension_from_index(_ctx, domain, i, &dim),
            _ctx,
            "TileDB failed to retrieve dimension object");

        void* tile_ext;
        Error_Check(
            tiledb_dimension_get_tile_extent(_ctx, dim, &tile_ext),
            _ctx,
            "TileDB failed to retrieve tile extent from dimension object");
        uint64_t* tui64 = static_cast<uint64_t*>(tile_ext);
        _tile_dimension.push_back(*tui64);


        void* domainptr;
        Error_Check(
            tiledb_dimension_get_domain(_ctx, dim, &domainptr),
            _ctx,
            "TileDB failed to retrieve domain values from dimension object");
        uint64_t* dom_tui64 = static_cast<uint64_t*>(domainptr);
        _array_dimension.push_back(dom_tui64[1] + 1);

        Error_Check(
            tiledb_dimension_free(_ctx, &dim), _ctx,
            "TileDB failed to free dimension object");
    }

    Error_Check(
        tiledb_domain_free(_ctx, &domain), _ctx,
        "TileDB failed to free domain object");

    Error_Check(
        tiledb_array_schema_free(_ctx, &array_schema),
        _ctx,
        "TileDB schema failed to be freed");
}


    /*  *********************** */
    /*   METADATA INTERACTION   */
    /*  *********************** */

void TDBObject::write_metadata(const std::string &metadata, const std::vector<std::string> &keys, 
            const std::vector<uint64_t> &values)
{
    tiledb_kv_schema_t* kv_schema;
    Error_Check(
        tiledb_kv_schema_create(_ctx, &kv_schema),
        _ctx,
        "TileDB failed to create metadata schema");

    for (int i = 0; i < keys.size(); ++i) {
        tiledb_attribute_t* attr;
        Error_Check(
            tiledb_attribute_create(_ctx, &attr, keys[i].c_str(), TILEDB_UINT64),
            _ctx,
            "TileDB failed to create metadata attribute");
        Error_Check(
            tiledb_attribute_set_cell_val_num(_ctx, attr, 1),
            _ctx,
            "TileDB failed set metadata attribute information");
        Error_Check(
            tiledb_attribute_set_compressor(_ctx, attr, convert_to_tiledb(), -1),
            _ctx,
            "TileDB failed set metadata attribute information");
        Error_Check(
            tiledb_kv_schema_add_attribute(_ctx, kv_schema, attr),
            _ctx,
            "TileDB failed to add metadata attribute to metadata schema");
        Error_Check(
            tiledb_attribute_free(_ctx, &attr), _ctx,
            "TileDB failed to free metadata attribute");
    }

    Error_Check(
        tiledb_kv_schema_check(_ctx, kv_schema), _ctx,
        "TileDB key value schema invalid");

    tiledb_object_t type;
    Error_Check(
        tiledb_object_type(_ctx, metadata.c_str(), &type),
        _ctx,
        "TileDB object check of " + metadata + " failed");
    if ( type != TILEDB_KEY_VALUE ) {
        Error_Check(
            tiledb_kv_create(_ctx, metadata.c_str(), kv_schema),
            _ctx,
            "Cannot create the TileDB directory " + metadata +
            "; directory may already exist or parent directory is not a TileDB directory");
    }

    Error_Check(
        tiledb_kv_schema_free(_ctx, &kv_schema), _ctx,
        "TileDB schema failed to be freed");

    tiledb_kv_item_t* kv_item;
    const char* keyname = _name.c_str();
    Error_Check(
        tiledb_kv_item_create(_ctx, &kv_item), _ctx,
        "TileDB failed to create metadata item");
    Error_Check(
        tiledb_kv_item_set_key(_ctx, kv_item, keyname, TILEDB_CHAR, std::strlen(keyname)),
        _ctx,
        "TileDB failed to set metadata key");

    for (int i = 0; i < keys.size(); ++i) {
        const char* key = keys[i].c_str();
        const void* val = &(values[i]);
        Error_Check(
            tiledb_kv_item_set_value(_ctx, kv_item, key, val, TILEDB_UINT64, sizeof(val)),
            _ctx,
            "TileDB failed to set metadata value");
    }
    
    tiledb_kv_t* kv;

    Error_Check( 
        tiledb_kv_open(_ctx, &kv, metadata.c_str(), NULL, 0),
        _ctx,
        "TileDB metadata open failed");

    Error_Check( 
        tiledb_kv_add_item(_ctx, kv, kv_item), _ctx,
        "TileDB metadata failed to write");

    Error_Check( 
        tiledb_kv_close(_ctx, &kv), _ctx,
        "TileDB metadata failed to close");

    Error_Check(
        tiledb_kv_item_free(_ctx, &kv_item), _ctx,
        "TileDB failed to free metadata item");
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

tiledb_compressor_t TDBObject::convert_to_tiledb()
{
    switch(static_cast<int>(_compressed)) {
        case 0:
            return TILEDB_NO_COMPRESSION;
        case 1:
            return TILEDB_GZIP;
        case 2:
            return TILEDB_ZSTD;
        case 3:
            return TILEDB_LZ4;
        case 4:
            return TILEDB_BLOSC_LZ;
        case 5:
            return TILEDB_BLOSC_LZ4;
        case 6:
            return TILEDB_BLOSC_LZ4HC;
        case 7:
            return TILEDB_BLOSC_SNAPPY;
        case 8:
            return TILEDB_BLOSC_ZLIB;
        case 9:
            return TILEDB_BLOSC_ZSTD;
        case 10:
            return TILEDB_RLE;
        case 11:
            return TILEDB_BZIP2;
        case 12:
            return TILEDB_DOUBLE_DELTA;
        default:
            throw VCLException(TileDBError, "Compression type not supported.\n");
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



