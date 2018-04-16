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

#include "TDBImage.h"
#include "TDBObject.h"
#include "VCL.h"

using namespace VCL;

void Polygon::create_edge_list(const std::vector<Point> &coords)
{
    float dx, dy;
    int sign, x_val, y_min, y_max;

    int size = coords.size();

    for ( int x = 0; x < size; ++x ) {
        Point cur_point, next_point;
        PolygonEdge edge;

        if ( x == 0 ) {
            cur_point = coords[size - 1];
            next_point = coords[x];
        }
        else if ( size > 2 ) {
            cur_point = coords[x - 1];
            next_point = coords[x];
        }
        else 
            break;

        dx = next_point.x - cur_point.x;
        dy = next_point.y - cur_point.y;

        if ( dy == 0 ) 
            continue;

        if ( dx < 0 || dy < 0 )
            edge.sign = -1;
        else
            edge.sign = 1;

        edge.dx = std::abs(dx);
        edge.dy = std::abs(dy);

        if ( cur_point.y < next_point.y ) {
            edge.y_min = cur_point.y;
            edge.y_max = next_point.y;
            edge.x_val = cur_point.x;
        }
        else {
            edge.y_min = next_point.y;
            edge.y_max = cur_point.y;
            edge.x_val = next_point.x;
        }

        edge.current_sum = 0;
        if ( !in_edges(edge) )
            _edges.push_back(edge);
    }

    sort_edges();
}

void Polygon::print_edge(PolygonEdge e)
{
    std::cout << "x_val = " << e.x_val << " y_min = " << e.y_min << " y_max = " << e.y_max;
    std::cout << " dx = " << e.dx << " dy = " << e.dy << " sign = " << e.sign;
    std::cout << " current sum = " << e.current_sum << std::endl;
}

bool Polygon::in_edges(PolygonEdge edge)
{
    for ( auto e : _edges ) {
        if (edge.x_val == e.x_val && edge.y_min == e.y_min && edge.y_max == e.y_max 
            && edge.dx == e.dx && edge.dy == e.dy && edge.sign == e.sign 
            && edge.current_sum == e.current_sum)
            return true;
    }
    return false;
}

void Polygon::print_edges()
{
    for ( auto e : _edges )
        print_edge(e);
}

void Polygon::manage_active(int current_y) 
{
    if ( !_active.empty() ) {
        for ( auto it = _active.begin(); it != _active.end(); ) {
            PolygonEdge edge = *it;
            if ( current_y == it->y_max ) {
                it = _active.erase(it);
            }
            else
                ++it;
        }
    }

    if ( !_edges.empty() ) {
        for ( auto it = _edges.begin(); it != _edges.end(); ) {
            PolygonEdge edge = *it;
            if ( current_y == edge.y_min) {
                _active.push_back(edge);
                it = _edges.erase(it);
            }
            else
                ++it;
        }
    }

    if ( !_active.empty() ) 
        std::sort(_active.begin(), _active.end(), xMinCompare);
}

void Polygon::sort_edges()
{
    std::sort(_edges.begin(), _edges.end(), yMaxCompare);
    std::sort(_edges.begin(), _edges.end(), yMinCompare);
}

void Polygon::get_subarray(uint64_t *subarray)
{
    int index = _edges.size() - 1;

    std::sort(_edges.begin(), _edges.end(), xMinCompare);
    uint64_t start_column = _edges[0].x_val;
    uint64_t end_column = _edges[index].x_val;

    std::sort(_edges.begin(), _edges.end(), yMinCompare);
    uint64_t start_row = _edges[0].y_min;
    uint64_t end_row = _edges[index].y_max;

    subarray[0] = start_row;
    subarray[1] = end_row - 1;
    subarray[2] = start_column;
    subarray[3] = end_column - 1;
}

void Polygon::get_active_x(std::vector<int> &x_vals)
{
    for ( auto &edge : _active )
        x_vals.push_back(edge.x_val);
}

void Polygon::swap_parity()
{
    if ( _parity == 0 ) 
        _parity = 1;
    else
        _parity = 0;
}

int Polygon::get_parity() {
    return _parity;
}

void Polygon::set_parity(int p) {
    _parity = p;
}

void Polygon::update()
{
    for ( auto &edge : _active ) {
        edge.current_sum += edge.dx;

        while ( edge.current_sum >= edge.dy ) {
            edge.current_sum -= edge.dy;
            edge.x_val += edge.sign; 
        }
    }
}

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

TDBImage::TDBImage(const std::string &image_id, RemoteConnection &connection) 
    : TDBObject(image_id, connection)
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
    uint64_t subarray[] = {0, _img_height - 1, 0, _img_width - 1};

    if ( _tile_order )
        reorder_buffer(buffer, subarray);
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

    uint64_t subarray[] = {0, _img_height - 1, 0, _img_width - 1};

    if ( _tile_order ) 
        reorder_buffer(buffer, subarray);
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

void TDBImage::set_configuration(RemoteConnection &remote) 
{
    if ( !remote.connected() )
        throw VCLException(SystemNotFound, "Remote Connection not initialized");
    
    set_config(remote);
}


    /*  *********************** */
    /*    TDBIMAGE INTERACTION  */
    /*  *********************** */
void TDBImage::write(const std::string &image_id, bool metadata)
{
    if ( _raw_data == NULL )
        throw VCLException(ObjectEmpty, "No data to be written");

    std::string array_name = namespace_setup(image_id);
    array_setup(metadata);

    tiledb_query_t* write_array;

    Error_Check(
        tiledb_query_create(_ctx, &write_array, 
            array_name.c_str(), TILEDB_WRITE),
        _ctx,
        "Failed to set up TileDB write");

    if ( _tile_order ) {
        Error_Check(
            tiledb_query_set_layout(_ctx, write_array, TILEDB_GLOBAL_ORDER),
            _ctx,
            "Failed to set TileDB layout");
    }
    else {
        Error_Check(
            tiledb_query_set_layout(_ctx, write_array, TILEDB_ROW_MAJOR),
            _ctx,
            "Failed to set TileDB layout");
    }


    if ( _num_attributes == 1 ) {
        // Size of buffers is equal to the number of attributes
        size_t buffer_size = _img_height * _img_width * _img_channels;
        void* buffers[] = { _raw_data };
        size_t buffer_sizes[] = { buffer_size };

        Error_Check(
            tiledb_query_set_buffers(_ctx, write_array, &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "TileDB buffer setup for write failed");
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
        void* buffers[] = { blue_buffer, green_buffer, red_buffer };
        size_t buffer_sizes[] = { buffer_size, buffer_size, buffer_size };

        Error_Check(
            tiledb_query_set_buffers(_ctx, write_array, &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "TileDB buffer setup for write failed");

    }

    Error_Check(
        tiledb_query_submit(_ctx, write_array), 
        _ctx,
        "TileDB array write failed");

    Error_Check(
        tiledb_query_free(_ctx, &write_array),
        _ctx,
        "Failed to free TileDB write request");
}


void TDBImage::write(const cv::Mat &cv_img, bool metadata)
{
    _tile_order = false;

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

    std::string array_name = _group + _name;
    array_setup(metadata);

    tiledb_query_t* write_array;

    Error_Check(
        tiledb_query_create(_ctx, &write_array, 
            array_name.c_str(), TILEDB_WRITE),
        _ctx,
        "Failed to set up TileDB write");

    Error_Check(
        tiledb_query_set_layout(_ctx, write_array, TILEDB_ROW_MAJOR),
        _ctx,
        "Failed to set TileDB layout");

    size_t buffer_size = _img_height * _img_width * _img_channels;

    if ( _num_attributes == 1 ) {
        _raw_data = new unsigned char[buffer_size];
        std::memcpy(_raw_data, cv_img.data, buffer_size);

        // Size of buffers is equal to the number of attributes
        void* buffers[] = { _raw_data };
        size_t buffer_sizes[] = { buffer_size };

        Error_Check(
            tiledb_query_set_buffers(_ctx, write_array, &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "TileDB buffer setup for write failed");
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

        // Size of buffers is equal to the number of attributes
        void* buffers[] = { blue_buffer, green_buffer, red_buffer };
        size_t buffer_sizes[] = { size, size, size };

        Error_Check(
            tiledb_query_set_buffers(_ctx, write_array, &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "TileDB buffer setup for write failed");

        delete [] blue_buffer;
        delete [] green_buffer;
        delete [] red_buffer;
    }

    Error_Check(
        tiledb_query_submit(_ctx, write_array), 
        _ctx,
        "TileDB array write failed");

    Error_Check(
        tiledb_query_free(_ctx, &write_array),
        _ctx,
        "Failed to free TileDB write request");
}

void TDBImage::read()
{
    if ( _raw_data == NULL )
    {
        if ( _img_height == 0 )
            read_metadata();

        uint64_t start_row = 0;
        uint64_t start_column = 0;
        uint64_t end_row = _img_height - 1;
        uint64_t end_column = _img_width - 1;

        uint64_t subarray[] = { start_row, end_row, start_column, end_column };

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

        uint64_t start_row = rect.x;
        uint64_t start_column = rect.y;
        uint64_t end_row = start_row + rect.height - 1;
        uint64_t end_column = start_column + rect.width - 1;

        uint64_t subarray[] = { start_row, end_row, start_column, end_column };

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
    std::vector<uint64_t> values = {_img_height, _img_width};
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

void TDBImage::area(const std::vector<Point> &coords, int fill_color)
{
    _poly.create_edge_list(coords);
    _poly.set_parity(0);

    uint64_t subarray[4];
    _poly.get_subarray(subarray);

    if (_raw_data == NULL) {

        if ( _img_height == 0 )
            read_metadata();

        if ( _img_height < subarray[1] - subarray[0] || _img_width < subarray[3] - subarray[2] )
            throw VCLException(SizeMismatch, "Requested area is not within the image");

        read_from_tdb(subarray);
    }

    // if the area requested is the entire image, don't bother trying to fill 
    if ( (coords.size() == 4) && 
        (_img_height == subarray[1] - subarray[0] + 1 && 
         _img_width == subarray[3] - subarray[2] + 1) )
        return;
    else
        fill_ordered_polygon(subarray, fill_color);
}

void TDBImage::fill_ordered_polygon(uint64_t* subarray, int fill_color)
{
    int i,j;
    std::vector<int> edges;
    int buffer_index = 0;
    int height = subarray[1] + 1;
    int width = subarray[3] + 1;
    int img_size = height * width * _img_channels;

    unsigned char* buffer = new unsigned char[img_size];
    reorder_buffer(buffer, subarray);

    for ( i = 0; i < height; ++i ) { 
        edges.clear();
        _poly.set_parity(0);
        _poly.manage_active(i);
        _poly.get_active_x(edges);

        if ( edges.back() != width )
            edges.push_back(width);

        int start = 0;
        int size = 0;

        for ( int n = 0; n < edges.size(); ++n ) {
            int parity = _poly.get_parity();
            size = edges[n] * _img_channels - start;

            if ( parity == 0 ) {
                memset(&buffer[buffer_index], fill_color, size);
            }

            buffer_index += size;
            start = edges[n] * _img_channels;

            _poly.swap_parity();
        }
        _poly.update();
    }

    if (_raw_data != NULL)
        delete [] _raw_data;
    _raw_data = new unsigned char[img_size];
    std::memcpy(_raw_data, buffer, img_size);

    delete [] buffer;
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
void TDBImage::get_tile_coordinates(uint64_t* subarray, int current_row_tile, int current_column_tile)
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
std::string TDBImage::namespace_setup(const std::string &image_id)
{
    size_t pos = get_path_delimiter(image_id);

    _group = get_group(image_id, pos);
    _name = get_name(image_id, pos);

    return _group + _name;
}

void TDBImage::array_setup(bool metadata)
{
    int num_values = 0;
    if ( _num_attributes == 1 && _img_channels == 3)
        num_values = 3;
    else
        num_values = 1;

    set_schema_dense(num_values, _group + _name);

    if (metadata) {
        std::vector<std::string> keys = {"height", "width", "channels"};
        std::vector<uint64_t> values = {_img_height, _img_width, _img_channels};

        std::string md_name = _group + _name + "/metadata";

        write_metadata(md_name, keys, values);
    }
}


    /*  *********************** */
    /*   METADATA INTERACTION   */
    /*  *********************** */
void TDBImage::read_metadata()
{
    tiledb_object_t group_type;
    tiledb_object_type(_ctx, _group.c_str(), &group_type);
    if ( group_type != TILEDB_GROUP )
        throw VCLException(TileDBNotFound, "Not a TileDB object");

    tiledb_kv_t* metadata_kv;
    std::string md_name = _group + _name + "/metadata";

    tiledb_object_t md_type;
    tiledb_object_type(_ctx, md_name.c_str(), &md_type);
    if ( md_type != TILEDB_KEY_VALUE )
        throw VCLException(TileDBNotFound, "Not a TileDB object");

    Error_Check(
        tiledb_kv_open(_ctx, &metadata_kv, md_name.c_str(),
            NULL, 0),
        _ctx,
        "TileDB metadata failed to open");

    const char* key = _name.c_str();
    tiledb_datatype_t key_type = TILEDB_CHAR;
    uint64_t key_size = std::strlen(key);

    tiledb_kv_item_t* kv_item = NULL;
    Error_Check(
        tiledb_kv_get_item(_ctx, metadata_kv, &kv_item, key, key_type, key_size),
        _ctx,
        "TileDB cannot retrieve metadata object");

    const void *h, *w, *c;
    tiledb_datatype_t h_type, w_type, c_type;
    uint64_t h_size, w_size, c_size;

    Error_Check(
        tiledb_kv_item_get_value(_ctx, kv_item, "height", &h, &h_type, &h_size),
        _ctx,
        "TileDB cannot retrieve height from metadata");
    Error_Check(
        tiledb_kv_item_get_value(_ctx, kv_item, "width", &w, &w_type, &w_size),
        _ctx,
        "TileDB cannot retrieve width from metadata");
    Error_Check(
        tiledb_kv_item_get_value(_ctx, kv_item, "channels", &c, &c_type, &c_size),
        _ctx,
        "TileDB cannot retrieve channels from metadata");

    _img_height = *((const uint64_t*)h);
    _dimension_values.push_back(_img_height);

    _img_width = *((const uint64_t*)w);
    _dimension_values.push_back(_img_width);

    _img_channels = *((const uint64_t*)c);

    _img_size = _img_height * _img_width * _img_channels;

    Error_Check(
        tiledb_kv_close(_ctx, &metadata_kv),
        _ctx,
        "TileDB metadata failed to close");
    Error_Check(
        tiledb_kv_item_free(_ctx, &kv_item),
        _ctx,
        "TileDB failed to free metadata object");
}


    /*  *********************** */
    /*     DATA MANIPULATION    */
    /*  *********************** */
void TDBImage::read_from_tdb(uint64_t* subarray)
{
    std::string array_name = _group + _name;

    size_t buffer_size = _img_size;
    _raw_data = new unsigned char[buffer_size];

    _tile_order = true;

    set_from_schema(array_name);

    tiledb_query_t* read_array;

    Error_Check(
        tiledb_query_create(_ctx, &read_array, array_name.c_str(), TILEDB_READ), 
        _ctx,
        "Failed to initialize TileDB array for reading");

    Error_Check(
        tiledb_query_set_layout(_ctx, read_array, TILEDB_GLOBAL_ORDER), 
        _ctx,
        "Failed to initialize TileDB array layout for reading");


    if ( _num_attributes == 1 ) {
        unsigned char* buffer = new unsigned char[buffer_size];
        void* buffers[] = { buffer }; // Size of buffers is equal to the number of attributes
        size_t buffer_sizes[] = { buffer_size };

        Error_Check(
            tiledb_query_set_buffers(_ctx, read_array,  &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "Failed to set buffers for reading from TileDB array");

        Error_Check(
            tiledb_query_submit(_ctx, read_array), 
            _ctx,
            "Failed to read from TileDB array");

        std::memcpy(_raw_data, buffer, buffer_size);

        Error_Check(
            tiledb_query_free(_ctx, &read_array),
            _ctx,
            "TileDB array failed close");

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

        Error_Check(
            tiledb_query_set_buffers(_ctx, read_array,  &_attributes[0], 
                _num_attributes, buffers, buffer_sizes),
            _ctx,
            "Failed to set buffers for reading from TileDB array");

        Error_Check(
            tiledb_query_submit(_ctx, read_array), 
            _ctx,
            "Failed to read from TileDB array");

        int count = 0;
        for (int i = 0; i < size; ++i) {
            _raw_data[count] = blue_buffer[i];
            _raw_data[count + 1] = green_buffer[i];
            _raw_data[count + 2] = red_buffer[i];
            count += 3;
        }

        Error_Check(
            tiledb_query_free(_ctx, &read_array),
            _ctx,
            "TileDB array failed close");

        delete [] blue_buffer;
        delete [] green_buffer;
        delete [] red_buffer;

    }
}

std::vector<int> TDBImage::manage_tiles(int start_row, int start_column)
{
    std::vector<int> tiles;
    int nRows = _array_dimension[0] / float(_tile_dimension[0]);
    int nCols = _array_dimension[1] / float(_tile_dimension[1]);

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

    if ( start_row > _tile_dimension[0] ) {
        while ( start_row > _tile_dimension[0] * nRows )
            nRows -= 1;
    }

    if ( start_column > _tile_dimension[1] ) {
        while ( start_column > _tile_dimension[1] * nCols )
            nCols -= 1;
    }

    tiles.push_back(nRows);
    tiles.push_back(nCols);

    return tiles;
}

template <class T>
void TDBImage::reorder_buffer(T* buffer, uint64_t* subarray)
{
    int i,j;

    int current_height = subarray[1] - subarray[0] + 1;
    int current_width = subarray[3] - subarray[2] + 1;

    int buffer_index = 0;
    int full_row_tile = current_width * _tile_dimension[0] * _img_channels;

    std::vector<int> tiles = manage_tiles(subarray[0], subarray[2]);

    for ( i = 0; i < tiles[0]; ++i ) {
        for ( j = 0; j < tiles[1]; ++j ) {
            uint64_t subarray[4];
            get_tile_coordinates(subarray, i, j);

            int start = i * full_row_tile + j * _tile_dimension[1] * _img_channels;
            buffer_index = reorder_tile(buffer, subarray, buffer_index, start);
        }
    }
    _tile_order = false;
}

template void TDBImage::reorder_buffer(unsigned char* buffer, uint64_t* subarray);

template <class T>
int TDBImage::reorder_tile(T* buffer, uint64_t* subarray, int buffer_index,
    int start_index)
{
    int current_tile_height = subarray[1] - subarray[0];
    int current_tile_width = subarray[3] - subarray[2];

    int data_index;
    int x, y;

    for ( x = 0; x < current_tile_height; ++x ) {
        data_index = start_index + x * _img_width * _img_channels;
        memcpy(&buffer[data_index], &_raw_data[buffer_index], current_tile_width*_img_channels);
        buffer_index += current_tile_width * _img_channels;
        // for ( y = 0; y < current_tile_width * _img_channels; ++y ) {
        //     buffer[data_index] = T(_raw_data[buffer_index]);
        //     ++data_index;
        //     ++buffer_index;
        // }
    }
    return buffer_index;
}

template int TDBImage::reorder_tile(unsigned char* buffer, uint64_t* subarray,
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
