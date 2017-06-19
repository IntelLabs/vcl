/**
 * @file   TDBImage.cc
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
#include "TDBImage.h"
#include "TDBObject.h"
#include "VCL.h"

using namespace VCL;

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
TDBImage::TDBImage() : TDBObject()
{
    _img_height = 0;
    _img_width = 0;
    _img_channels = 0;
    _img_size = 0;

    _threshold = 0;

    set_num_dimensions(2);

    set_default_attributes();
    set_default_dimensions();

    _raw_data = NULL;
    _tile_order = false;
}

TDBImage::TDBImage(const std::string &image_id) : TDBObject(image_id)
{
    _img_height = 0;
    _img_width = 0;
    _img_channels = 0;
    _img_size = 0;

    _threshold = 0;

    set_num_dimensions(2);
    set_default_attributes();
    set_default_dimensions();

    _raw_data = NULL;
    _tile_order = false;
}

template <class T>
TDBImage::TDBImage(T* buffer, int size) : TDBObject()
{
    _img_height = 0;
    _img_width = 0;
    _img_channels = 0;
    _img_size = size;

    _threshold = 0;

    set_num_dimensions(2);
    set_default_attributes();
    set_default_dimensions();

    _raw_data = new unsigned char[size];
    std::memcpy(_raw_data, buffer, _img_size);
    _tile_order = false;
}

// OpenCV type CV_8UC1-4
template TDBImage::TDBImage(unsigned char* buffer, int size);
// OpenCV type CV_8SC1-4
template TDBImage::TDBImage(char* buffer, int size);
// OpenCV type CV_16UC1-4
template TDBImage::TDBImage(unsigned short* buffer, int size);
// OpenCV type CV_16SC1-4
template TDBImage::TDBImage(short* buffer, int size);
// OpenCV type CV_32SC1-4
template TDBImage::TDBImage(int* buffer, int size);
// OpenCV type CV_32FC1-4
template TDBImage::TDBImage(float* buffer, int size);
// OpenCV type CV_64FC1-4
template TDBImage::TDBImage(double* buffer, int size);


TDBImage::TDBImage(TDBImage &tdb) : TDBObject(tdb)
{
    if ( !tdb.has_data() ) {
        try {
            tdb.read();
        }
        catch ( VCL::Exception &e ) {
            _raw_data = NULL;
        }
    }

    set_equal(tdb);
    set_image_data_equal(tdb);

    if ( tdb.has_data() ) {
        _raw_data = new unsigned char[_img_size];
        std::memcpy(_raw_data, tdb._raw_data, _img_size);
    }
}

void TDBImage::operator=(TDBImage &tdb)
{
    unsigned char *temp = _raw_data;

    if ( !tdb.has_data() ) {
        try {
            tdb.read();
        }
        catch ( VCL::Exception &e ) {
            _raw_data = NULL;
        }
    }

    set_equal(tdb);
    set_image_data_equal(tdb);

    if ( tdb.has_data() ) {
        if (_raw_data != NULL)
            delete [] _raw_data;

        _raw_data = new unsigned char[_img_size];
        std::memcpy(_raw_data, tdb._raw_data, _img_size);
    }

    delete temp;
}

void TDBImage::set_image_data_equal(const TDBImage &tdb)
{
    _img_height = tdb._img_height;
    _img_width = tdb._img_width;
    _img_channels = tdb._img_channels;
    _img_size = tdb._img_size;
    _threshold = tdb._threshold;
    _tile_order = tdb._tile_order;
}

TDBImage::~TDBImage()
{
    delete [] _raw_data;
}


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

int TDBImage::get_image_size()
{
    if (_img_size == 0 && _name == "")
        throw VCLException(TileDBNotFound, "No data in TileDB object yet");
    else if (_img_size == 0 && _name != "")
        read_metadata();

    return _img_size;
}

int TDBImage::get_image_height()
{
    if (_img_height == 0 && _name == "")
        throw VCLException(TileDBNotFound, "No data in TileDB object yet");
    else if ( _img_height == 0 && _name != "")
        read_metadata();

    return _img_height;
}

int TDBImage::get_image_width()
{
    if (_img_width == 0 && _name == "")
        throw VCLException(TileDBNotFound, "No data in TileDB object yet");
    else if ( _img_width == 0 && _name != "")
        read_metadata();

    return _img_width;
}

int TDBImage::get_image_channels()
{
    if (_img_channels == 0 && _name == "")
        throw VCLException(TileDBNotFound, "No data in TileDB object yet");
    else if ( _img_channels == 0 && _name != "")
        read_metadata();

    return _img_channels;
}

cv::Mat TDBImage::get_cvmat()
{
    if ( _raw_data == NULL )
        read();

    unsigned char* buffer = new unsigned char[_img_size];

    if ( _tile_order )
        reorder_buffer(buffer);
    else
        std::memcpy(buffer, _raw_data, _img_size);

    cv::Mat img_clone;

    if ( _img_channels == 1 ) {
        cv::Mat img(cv::Size(_img_width, _img_height), CV_8UC1, buffer);
        img_clone = img.clone();
    }
    else {
        cv::Mat img(cv::Size(_img_width, _img_height), CV_8UC3, buffer);
        img_clone = img.clone();
    }

    delete [] buffer;
    return img_clone;
}

template <class T>
void TDBImage::get_buffer(T* buffer, int buffer_size)
{
    if ( buffer_size != get_image_size() )
        throw VCLException(SizeMismatch, buffer_size + " is not equal to "
            + get_image_size());

    if ( _raw_data == NULL )
        read();

    if ( _tile_order ) 
        reorder_buffer(buffer);
    else
        std::memcpy(buffer, _raw_data, buffer_size);
}

template void TDBImage::get_buffer(unsigned char* buffer, int buffer_size);
template void TDBImage::get_buffer(char* buffer, int buffer_size);
template void TDBImage::get_buffer(unsigned short* buffer, int buffer_size);
template void TDBImage::get_buffer(short* buffer, int buffer_size);
template void TDBImage::get_buffer(int* buffer, int buffer_size);
template void TDBImage::get_buffer(float* buffer, int buffer_size);
template void TDBImage::get_buffer(double* buffer, int buffer_size);


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */

void TDBImage::set_image_properties(int height, int width, int channels)
{
    _img_height = height;
    _img_width = width;
    _img_channels = channels;
    _img_size = _img_height * _img_width * _img_channels;
}



    /*  *********************** */
    /*    TDBIMAGE INTERACTION  */
    /*  *********************** */
void TDBImage::write(const std::string &image_id, bool metadata)
{
    if ( _raw_data == NULL )
        throw VCLException(ObjectEmpty, "No data to be written");

    std::string array_name = workspace_setup(image_id);
    int num_values = array_setup(array_name, metadata);

    TileDB_Array* image_array;

    if ( _tile_order ) {
        Error_Check(
            tiledb_array_init(_ctx, &image_array, array_name.c_str(),
                TILEDB_ARRAY_WRITE, NULL, NULL, 0),
            "TileDB array failed to initialize");
    }
    else {
        Error_Check(
            tiledb_array_init(_ctx, &image_array, array_name.c_str(),
                TILEDB_ARRAY_WRITE_SORTED_ROW, NULL, NULL, 0),
            "TileDB array failed to initialize");
    }


    if ( _num_attributes == 1 ) {
        // Size of buffers is equal to the number of attributes
        const void* buffers[] = { _raw_data };
        size_t buffer_sizes[] = { sizeof(_raw_data) };

        Error_Check(
            tiledb_array_write(image_array, buffers, buffer_sizes),
            "TileDB write to array failed");
    }
    else {
        size_t buffer_size = _img_height*_img_width;
        unsigned char* blue_buffer = new unsigned char[buffer_size];
        unsigned char* green_buffer = new unsigned char[buffer_size];
        unsigned char* red_buffer = new unsigned char[buffer_size];

        int count = 0;
        for ( int i = 0; i < buffer_size; ++i ) {
            blue_buffer[i] = _raw_data[count];
            green_buffer[i] = _raw_data[count + 1];
            red_buffer[i] = _raw_data[count + 2];
        }

        // Size of buffers is equal to the number of attributes
        const void* buffers[] = { blue_buffer, green_buffer, red_buffer };
        size_t buffer_sizes[] = { buffer_size, buffer_size, buffer_size };

        Error_Check(
            tiledb_array_write(image_array, buffers, buffer_sizes),
            "TileDB write to array failed");

    }

    Error_Check(
        tiledb_array_finalize(image_array),
        "TileDB array failed to finalize");
}


void TDBImage::write(const cv::Mat &cv_img, bool metadata)
{
    if ( _group == "" )
        throw VCLException(ObjectNotFound, "Object path is not defined");
    if ( _name == "" )
        throw VCLException(ObjectNotFound, "Object name is not defined");

    _dimension_values.push_back(cv_img.rows);
    _dimension_values.push_back(cv_img.cols);

    _img_height = cv_img.rows;
    _img_width = cv_img.cols;
    _img_channels = cv_img.channels();
    _img_size = _img_height * _img_width * _img_channels;

    std::string array_name = workspace_setup(_group + _name);
    int num_values = array_setup(array_name, metadata);

    TileDB_Array* image_array;
    Error_Check(
        tiledb_array_init(_ctx, &image_array, array_name.c_str(),
            TILEDB_ARRAY_WRITE_SORTED_ROW, NULL, NULL, 0),
        "TileDB array failed to initialize");

    size_t buffer_size = _img_height * _img_width * _img_channels;

    if ( _num_attributes == 1 ) {
        _raw_data = new unsigned char[buffer_size];
        std::memcpy(_raw_data, cv_img.data, buffer_size);
        _tile_order = false;

        // Size of buffers is equal to the number of attributes
        const void* buffers[] = { _raw_data };
        size_t buffer_sizes[] = { buffer_size };

        Error_Check(
            tiledb_array_write(image_array, buffers, buffer_sizes),
            "TileDB write to array failed");
    }
    else {
        std::vector<cv::Mat> channels(3);
        cv::split(cv_img, channels);
        size_t size = _img_height * _img_width;
        unsigned char* blue_buffer = new unsigned char[size];
        unsigned char* green_buffer = new unsigned char[size];
        unsigned char* red_buffer = new unsigned char[size];

        const unsigned char* bp;
        for ( int i = 0; i < _img_height; ++i ) {
            bp = channels[0].ptr<unsigned char>(i);
            unsigned char* b = &blue_buffer[i * _img_width];
            std::memcpy(b, bp, _img_width);
        }

        const unsigned char* gp;
        for ( int i = 0; i < _img_height; ++i ) {
            gp = channels[1].ptr<unsigned char>(i);
            unsigned char* g = &green_buffer[i * _img_width];
            std::memcpy(g, gp, _img_width);
        }

        const unsigned char* rp;
        for ( int i = 0; i < _img_height; ++i ) {
            rp = channels[2].ptr<unsigned char>(i);
            unsigned char* r = &red_buffer[i * _img_width];
            std::memcpy(r, rp, _img_width);
        }

        _raw_data = new unsigned char[buffer_size];
        std::memcpy(_raw_data, cv_img.data, buffer_size);
        _tile_order = false;

        // Size of buffers is equal to the number of attributes
        const void* buffers[] = { blue_buffer, green_buffer, red_buffer };
        size_t buffer_sizes[] = { size, size, size };

        Error_Check(
            tiledb_array_write(image_array, buffers, buffer_sizes),
            "TileDB write to array failed");

        delete [] blue_buffer;
        delete [] green_buffer;
        delete [] red_buffer;
    }

    Error_Check(
        tiledb_array_finalize(image_array),
        "TileDB array failed to finalize");
}

void TDBImage::read()
{
    if ( _raw_data == NULL )
    {
        if ( _img_height == 0 )
            read_metadata();

        int start_row = 0;
        int start_column = 0;
        int end_row = _img_height - 1;
        int end_column = _img_width - 1;

        int64_t subarray[] = { start_row, end_row, start_column, end_column };

        read_from_tdb(subarray);
    }
}

void TDBImage::read(const Rectangle &rect)
{
    if (_raw_data == NULL) {

    if ( _img_height == 0 )
        read_metadata();

    if ( _img_height < rect.height + rect.y || _img_width < rect.width + rect.x )
        throw VCLException(SizeMismatch, "Requested area is not within the image");

    _img_height = rect.height;
    _img_width = rect.width;
    _img_size = _img_height * _img_width * _img_channels;

    int start_row = rect.x;
    int start_column = rect.y;
    int end_row = start_row + rect.height - 1;
    int end_column = start_column + rect.width - 1;

    int64_t subarray[] = { start_row, end_row, start_column, end_column };

    read_from_tdb(subarray);
    }
}

void TDBImage::resize(const Rectangle &rect)
{
    if ( _raw_data == NULL )
        read();

    int r, c;

    int data_index = 0;
    unsigned char* image_buffer = new unsigned char[rect.height * rect.width * _img_channels];

    float row_ratio = _img_height / float(rect.height);
    float column_ratio = _img_width / float(rect.width);

    for ( r = 0; r < rect.height; ++r ) {
        float scale_r =  ( r + 0.5 ) * row_ratio - 0.5;

        for ( c = 0; c < rect.width; ++c ) {
            float scale_c =  ( c + 0.5 ) * column_ratio - 0.5;

            data_index = rect.width * r * _img_channels + c * _img_channels;

            get_index_value(image_buffer, data_index, scale_r, scale_c);
         }
    }

    _img_height = rect.height;
    _img_width = rect.width;
    _img_size = _img_height * _img_width * _img_channels;
    std::vector<int> values = {_img_height, _img_width};
    set_dimension_values(values);

    _raw_data = new unsigned char[_img_size];
    std::memcpy(_raw_data, image_buffer, _img_size);
    _tile_order = false;

    delete [] image_buffer;
}

void TDBImage::threshold(int value)
{
    if ( _raw_data == NULL ) {
        _threshold = value;
        read();
    }

    else {
        int length = _img_height * _img_width * _img_channels;

        for ( int i = 0; i < length; ++i ) {
            if ( _raw_data[i] <= value )
                _raw_data[i] = 0;
        }
    }
}

bool TDBImage::has_data()
{
    if ( _raw_data == NULL )
        return false;
    else
        return true;
}

void TDBImage::delete_image()
{
    delete _raw_data;
    _raw_data = NULL;
    delete_object();
}

    /*  *********************** */
    /*   PRIVATE GET FUNCTIONS  */
    /*  *********************** */
std::string TDBImage::get_parent_dir(const std::string &filename) const
{
    std::string delimiter = "/";
    size_t loc = filename.rfind(delimiter);
    std::string parent_dir = filename.substr(0, loc);

    loc = parent_dir.rfind(delimiter);
    return parent_dir.substr(0, loc + 1);
}



void TDBImage::get_tile_coordinates(int64_t* subarray, int current_row_tile, int current_column_tile)
{
    int row_start = current_row_tile * _tile_dimension[0];
    int column_start = current_column_tile * _tile_dimension[1];
    int row_end = row_start + _tile_dimension[0];
    int column_end = column_start + _tile_dimension[1];

    if (row_end > _img_height)
        row_end = (_img_height - row_start) + row_start;

    if (column_end > _img_width)
        column_end = (_img_width - column_start) + column_start;

    subarray[0] = row_start;
    subarray[1] = row_end;
    subarray[2] = column_start;
    subarray[3] = column_end;
}

void TDBImage::get_index_value(unsigned char* image_buffer, int index,
    float scale_r, float scale_c)
{
    int column_left = floor(scale_c);
    int column_right = floor(scale_c + 1);
    int row_top = floor(scale_r);
    int row_bottom = floor(scale_r + 1);

    if ( column_left < 0 )
        column_left = 0;
    if ( column_right > _img_width - 1 )
        column_right = _img_width - 1;

    if ( row_top < 0 )
        row_top = 0;
    if ( row_bottom > _img_height - 1 )
        row_bottom = _img_height - 1;

    int top_left_index = get_index(row_top, column_left) * _img_channels;
    int top_right_index = get_index(row_top, column_right) * _img_channels;
    int bottom_left_index = get_index(row_bottom, column_left) * _img_channels;
    int bottom_right_index = get_index(row_bottom, column_right) * _img_channels;

    for ( int x = 0; x < _img_channels; ++x ) {
        unsigned char top_left = _raw_data[top_left_index + x];
        unsigned char top_right = _raw_data[top_right_index + x];
        unsigned char bottom_left = _raw_data[bottom_left_index + x];
        unsigned char bottom_right = _raw_data[bottom_right_index + x];

        double top = linear_interpolation(column_left, top_left, column_right, top_right, scale_c);
        double bottom = linear_interpolation(column_left, bottom_left, column_right, bottom_right, scale_c);
        double middle = linear_interpolation(row_top, top, row_bottom, bottom, scale_r);

        // we want the middle of the pixel
        unsigned char pixel_value = floor(middle + 0.5);
        image_buffer[ index + x ] = pixel_value;
    }
}

int TDBImage::get_index(int row, int column) const
{
    int tile_width = get_tile_width(column, _img_width / _tile_dimension[1]);
    int tile_height = get_tile_height(row, _img_height / _tile_dimension[0]);

    int tile_size = tile_width * tile_height;

    int current_tile_row = row % int(_tile_dimension[0]);
    int current_row_tile = row / int(_tile_dimension[0]);
    int current_column_tile = column / int(_tile_dimension[1]);
    int current_tile_column = column % int(_tile_dimension[1]);

    int full_row_tile = _img_width * _tile_dimension[0];

    int row_index = current_row_tile * full_row_tile + current_tile_row * tile_width;
    int column_index = current_column_tile * tile_size + current_tile_column;

    return row_index + column_index;
}

int TDBImage::get_tile_height(int row, int number_tiles) const
{
    int tile_height = int(_tile_dimension[0]);

    if ( row / _tile_dimension[0] == number_tiles )
        tile_height = _img_height - (number_tiles) * _tile_dimension[0];

    return tile_height;
}

int TDBImage::get_tile_width(int column, int number_tiles) const
{
    int tile_width = int(_tile_dimension[1]);

    if ( column / _tile_dimension[1] == number_tiles )
        tile_width = _img_width - (number_tiles) * _tile_dimension[1];

    return tile_width;
}


    /*  *********************** */
    /*   PRIVATE SET FUNCTIONS  */
    /*  *********************** */
void TDBImage::set_default_dimensions()
{
    _dimension_names.push_back("height");
    _dimension_names.push_back("width");
}

void TDBImage::set_default_attributes()
{
    _attributes.clear();
    switch (_num_attributes) {
        case 1: {
            _attributes.push_back("pixel");
            break;
        }
        case 3: {
            _attributes.push_back("blue");
            _attributes.push_back("green");
            _attributes.push_back("red");
            break;
        }
    }
}


    /*  *********************** */
    /*      TDBIMAGE SETUP      */
    /*  *********************** */
std::string TDBImage::workspace_setup(const std::string &image_id)
{
    size_t pos = get_path_delimiter(image_id);

    std::string workspace = get_workspace(image_id, pos);

    std::string parent_dir = get_parent_dir(workspace);
    std::string group = get_group(image_id, pos);

    return group + get_name(image_id, pos);
}

int TDBImage::array_setup(const std::string &image_id, bool metadata)
{
    int num_values = 0;
    if ( _num_attributes == 1 && _img_channels == 3)
        num_values = 3;
    else
        num_values = 1;

    set_schema(num_values, image_id);

    if (metadata) {
        int64_t buffer[_num_dimensions + 1];
        size_t buffer_keys[_num_dimensions + 1];

        buffer[0] = _img_height;
        buffer[1] = _img_width;
        buffer[2] = _img_channels;

        buffer_keys[0] = 0;
        buffer_keys[1] = 5;
        buffer_keys[2] = 13;

        char buffer_var_keys[] = { "rows\0columns\0channels" };

        std::string md_name = image_id + "/metadata";

        write_metadata(md_name, buffer, buffer_var_keys, buffer_keys, sizeof(buffer_var_keys));
    }

    return num_values;
}


    /*  *********************** */
    /*   METADATA INTERACTION   */
    /*  *********************** */
void TDBImage::read_metadata()
{
    if ( tiledb_dir_type(_ctx, _group.c_str()) != TILEDB_GROUP )
        throw VCLException(TileDBNotFound, "Not a TileDB object");

    const char* attributes[] = { "dimensions" };
    int attrs = 1;

    TileDB_Metadata* md;
    std::string md_name = _group + _name + "/metadata";

    if ( tiledb_dir_type(_ctx, md_name.c_str()) != TILEDB_METADATA )
        throw VCLException(TileDBNotFound, "Not a TileDB object");

    Error_Check(
        tiledb_metadata_init(_ctx, &md, md_name.c_str(),
            TILEDB_METADATA_READ, attributes, attrs),
        "TileDB metadata failed to initialize");

    const char* keys[] = { "rows", "columns", "channels" };

    int64_t rbuffer[10];
    void* rbuffers[] = { rbuffer };
    size_t rbuffer_sizes[] = { sizeof(rbuffer) };

    Error_Check(
        tiledb_metadata_read(md, keys[0], rbuffers, rbuffer_sizes),
        "TileDB metadata read failed");
    _img_height = static_cast<int64_t*>(rbuffers[0])[0];
    _dimension_values.push_back(_img_height);

    int64_t cbuffer[10];
    void* cbuffers[] = { cbuffer };
    size_t cbuffer_sizes[] = { sizeof(cbuffer) };

    Error_Check(
        tiledb_metadata_read(md, keys[1], cbuffers, cbuffer_sizes),
        "TileDB metadata read failed");
    _img_width = static_cast<int64_t*>(cbuffers[0])[0];
    _dimension_values.push_back(_img_width);

    int64_t hbuffer[10];
    void* hbuffers[] = { hbuffer };
    size_t hbuffer_sizes[] = { sizeof(hbuffer) };

    Error_Check(
        tiledb_metadata_read(md, keys[2], hbuffers, hbuffer_sizes),
        "TileDB metadata read failed");
    _img_channels = static_cast<int64_t*>(hbuffers[0])[0];

    _img_size = _img_height * _img_width * _img_channels;

    Error_Check(
        tiledb_metadata_finalize(md),
        "TileDB metadata failed to finalize");
}


    /*  *********************** */
    /*     DATA MANIPULATION    */
    /*  *********************** */
void TDBImage::read_from_tdb(int64_t* subarray)
{
    std::string array_name = _group + _name;

    size_t buffer_size = _img_size;
    _raw_data = new unsigned char[buffer_size];

    TileDB_Array* tiledb_array;
    Error_Check(
        tiledb_array_init(_ctx, &tiledb_array, array_name.c_str(),
            TILEDB_ARRAY_READ, subarray, NULL, 0),
        "TileDB array initialization failed");

    set_from_schema(tiledb_array);

    if ( _num_attributes == 1 ) {
        unsigned char* buffer = new unsigned char[buffer_size];
        void* buffers[] = { buffer }; // Size of buffers is equal to the number of attributes
        size_t buffer_sizes[] = { buffer_size };

        int overflow = 0;
        do {
            Error_Check(
                tiledb_array_read(tiledb_array, buffers, buffer_sizes),
                "TileDB read failed");
            overflow = tiledb_array_overflow(tiledb_array, 0);
            Error_Check(overflow, "TileDB Array Overflow error");
        } while ( overflow == 1);

        std::memcpy(_raw_data, buffer, buffer_size);

        Error_Check(
            tiledb_array_finalize(tiledb_array),
            "TileDB array failed to finalize");

        _tile_order = true;

        delete [] buffer;
    }

    else {
        size_t size = _img_height * _img_width;
        unsigned char* blue_buffer = new unsigned char[size];
        unsigned char* green_buffer = new unsigned char[size];
        unsigned char* red_buffer = new unsigned char[size];

        // Size of buffers is equal to the number of attributes
        void* buffers[] = { blue_buffer, green_buffer, red_buffer };
        size_t buffer_sizes[] = { size, size, size };

        int overflow = 0;
        do {
            Error_Check(
                tiledb_array_read(tiledb_array, buffers, buffer_sizes),
                "TileDB read failed");
            overflow = tiledb_array_overflow(tiledb_array, 0);
            Error_Check(overflow, "TileDB Array Overflow error");
        } while ( overflow == 1);

        int count = 0;
        for (int i = 0; i < size; ++i) {
            _raw_data[count] = blue_buffer[i];
            _raw_data[count + 1] = green_buffer[i];
            _raw_data[count + 2] = red_buffer[i];
            count += 3;
        }

        Error_Check(
            tiledb_array_finalize(tiledb_array),
            "TileDB array failed to finalize");

        _tile_order = true;

        delete [] blue_buffer;
        delete [] green_buffer;
        delete [] red_buffer;

    }
}


template <class T>
void TDBImage::reorder_buffer(T* buffer)
{
    int i,j;
    int nRows = _array_dimension[0] / float(_tile_dimension[0]);
    int nCols = _array_dimension[1] / float(_tile_dimension[1]);

    int buffer_index = 0;
    int full_row_tile = _img_width * _tile_dimension[0] * _img_channels;

    if ( _array_dimension[1] > _img_width ) {
        nCols = _img_width / _tile_dimension[1];
        if ( nCols * _tile_dimension[1] < _img_width )
            nCols += 1;
    }
    if ( _array_dimension[0] > _img_height ) {
        nRows = _img_height / _tile_dimension[0];
        if ( nRows * _tile_dimension[0] < _img_height )
            nRows += 1;
    }

    for ( i = 0; i < nRows; ++i ) {
        for ( j = 0; j < nCols; ++j ) {
            int64_t subarray[4];
            get_tile_coordinates(subarray, i, j);

            int start = i * full_row_tile + j * _tile_dimension[1] * _img_channels;
            buffer_index = reorder_tile(buffer, subarray, buffer_index, start);
        }
    }
}

template void TDBImage::reorder_buffer(unsigned char* buffer);

template <class T>
int TDBImage::reorder_tile(T* buffer, int64_t* subarray, int buffer_index,
    int start_index)
{
    int current_tile_height = subarray[1] - subarray[0];
    int current_tile_width = subarray[3] - subarray[2];

    int data_index;
    int x, y;

    for ( x = 0; x < current_tile_height; ++x ) {
        data_index = start_index + x * _img_width * _img_channels;
        for ( y = 0; y < current_tile_width * _img_channels; ++y ) {
            buffer[data_index] = T(_raw_data[buffer_index]);
            ++data_index;
            ++buffer_index;
        }
    }
    return buffer_index;
}

template int TDBImage::reorder_tile(unsigned char* buffer, int64_t* subarray,
        int buffer_index, int start_index);


    /*  *********************** */
    /*      MATH FUNCTIONS      */
    /*  *********************** */
double TDBImage::linear_interpolation(double x1, double val1, double x2,
    double val2, double x)
{

    if ( x1 == x2 )
        return val1;

    double value = val2 - val1;
    double multiply = x - x1;
    double divide = x2 - x1;

    return val1 + (value/divide * multiply);
}
