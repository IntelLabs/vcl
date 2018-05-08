/**
 * @file   ImageData.cc
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
 *
 */

#include <sys/stat.h>
#include <cstdio>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "ImageData.h"


using namespace VCL;

    /*  *********************** */
    /*        OPERATION         */
    /*  *********************** */

    /*  *********************** */
    /*       READ OPERATION     */
    /*  *********************** */
ImageData::Read::Read(const std::string& filename, ImageFormat format)
    : Operation(format),
      _fullpath(filename)
{
}

void ImageData::Read::operator()(ImageData *img)
{
    if ( _format == VCL::TDB ) {
        if ( img->_tdb == NULL )
            throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                stored in TDB format, but no data was found");

        img->_tdb->read();
        img->_height = img->_tdb->get_image_height();
        img->_width = img->_tdb->get_image_width();
        img->_channels = img->_tdb->get_image_channels();
    }
    else {
        img->copy_cv(cv::imread(_fullpath, cv::IMREAD_ANYCOLOR));
        if ( img->_cv_img.empty() )
            throw VCLException(ObjectEmpty, _fullpath + " could not be read, \
                object is empty");
    }
}

    /*  *********************** */
    /*       WRITE OPERATION    */
    /*  *********************** */
ImageData::Write::Write(const std::string& filename, ImageFormat format,
    ImageFormat old_format, bool metadata)
    : Operation(format),
      _old_format(old_format),
      _metadata(metadata),
      _fullpath(filename)
{
}

void ImageData::Write::operator()(ImageData *img)
{
    if (_format == VCL::TDB) {
        if ( img->_tdb == NULL ) {
            img->_tdb = new TDBImage(_fullpath);
            img->_tdb->set_compression(img->_compress);
        }

        if ( img->_tdb->has_data() )
            img->_tdb->write(_fullpath, _metadata);
        else
            img->_tdb->write(img->_cv_img, _metadata);
    }
    else {
        cv::Mat cv_img;
        if (_old_format == VCL::TDB)
            cv_img = img->_tdb->get_cvmat();
        else
            cv_img = img->_cv_img;

        if ( !cv_img.empty() )
            cv::imwrite(_fullpath, cv_img);

        else
            throw VCLException(ObjectEmpty, _fullpath + " could not be written \
                object is empty");
    }
}

    /*  *********************** */
    /*       RESIZE OPERATION   */
    /*  *********************** */

void ImageData::Resize::operator()(ImageData *img)
{
    if ( _format == VCL::TDB ) {
        img->_tdb->resize(_rect);
        img->_height = img->_tdb->get_image_height();
        img->_width = img->_tdb->get_image_width();
        img->_channels = img->_tdb->get_image_channels();
    }
    else {
        if ( !img->_cv_img.empty() ) {
            cv::Mat cv_resized;
            cv::resize(img->_cv_img, cv_resized, cv::Size(_rect.width, _rect.height));
            img->copy_cv(cv_resized);
        }
        else
            throw VCLException(ObjectEmpty, "Image object is empty");
    }
}

    /*  *********************** */
    /*       CROP OPERATION     */
    /*  *********************** */

void ImageData::Crop::operator()(ImageData *img)
{
    if ( _format == VCL::TDB ) {
        img->_tdb->read(_rect);
        img->_height = img->_tdb->get_image_height();
        img->_width = img->_tdb->get_image_width();
        img->_channels = img->_tdb->get_image_channels();
    }
    else {
        if ( !img->_cv_img.empty() ) {
            if ( img->_cv_img.rows < _rect.height + _rect.y || img->_cv_img.cols < _rect.width + _rect.x )
                throw VCLException(SizeMismatch, "Requested area is not within the image");
            cv::Mat roi_img(img->_cv_img, _rect);
            img->copy_cv(roi_img);
        }
        else
            throw VCLException(ObjectEmpty, "Image object is empty");
    }
}

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */

void ImageData::Threshold::operator()(ImageData *img)
{
    if ( _format == VCL::TDB )
        img->_tdb->threshold(_threshold);
    else {
        if ( !img->_cv_img.empty() )
            cv::threshold(img->_cv_img, img->_cv_img, _threshold, _threshold,
                cv::THRESH_TOZERO);
        else
            throw VCLException(ObjectEmpty, "Image object is empty");
    }
}


                    /*  *********************** */
                    /*         IMAGEDATA        */
                    /*  *********************** */

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */

ImageData::ImageData()
{
    _channels = 0;
    _height = 0;
    _width = 0;
    _cv_type = CV_8UC3;

    _format = VCL::NONE;
    _compress = VCL::CompressionType::LZ4;

    _tdb = NULL;
    _image_id = "";
}

ImageData::ImageData(const cv::Mat &cv_img)
{
    copy_cv(cv_img);

    _format = VCL::NONE;
    _compress = VCL::CompressionType::LZ4;
    _image_id = "";

    _tdb = NULL;
}

ImageData::ImageData(const std::string &image_id)
{
    _channels = 0;
    _height = 0;
    _width = 0;
    _cv_type = CV_8UC3;

    std::string extension = get_extension(image_id);
    set_format(extension);

    _compress = VCL::CompressionType::LZ4;

    _image_id = create_fullpath(image_id, _format);

    if ( _format == VCL::TDB ) {
        _tdb = new TDBImage(_image_id);
        _tdb->set_compression(_compress);
    }
    else
        _tdb = NULL;

}

ImageData::ImageData(void* buffer, cv::Size dimensions, int cv_type)
{
    _height = dimensions.height;
    _width = dimensions.width;
    _cv_type = cv_type;
    _channels = (cv_type / 8) + 1;

    _format = VCL::TDB;
    _compress = VCL::CompressionType::LZ4;
    _image_id = "";

    set_data_from_raw(buffer, _height*_width*_channels);
    _tdb->set_compression(_compress);
}

ImageData::ImageData(const ImageData &img)
{
    _format = img._format;
    _compress = img._compress;
    _image_id = img._image_id;

    if ( !(img._cv_img).empty() )
        copy_cv(img._cv_img);

    if ( img._tdb != NULL )
        _tdb = new TDBImage(*img._tdb);
    else
        _tdb = NULL;

    int start;
    if ( img._operations.size() > 0 ) {
        std::shared_ptr<Operation> front = img._operations.front();
        if (front->get_type() == OperationType::READ) {
            start = 1;
            copy_cv(cv::imread(img._image_id, cv::IMREAD_ANYCOLOR));
        }
        else
            start = 0;

        for (int i = start; i < img._operations.size(); ++i)
            _operations.push_back(img._operations[i]);
    }
}

void ImageData::operator=(const ImageData &img)
{
    TDBImage *temp = _tdb;

    if ( !(img._cv_img).empty() )
        copy_cv(img._cv_img);
    else {
        _channels = img._channels;

        _height = img._height;
        _width = img._width;

        _cv_type = img._cv_type;
    }

    _format = img._format;
    _compress = img._compress;
    _image_id = img._image_id;

    if ( img._tdb != NULL ) {
        _tdb = new TDBImage(*img._tdb);
    }
    else
        _tdb = NULL;

    int start;

    _operations.clear();
    _operations.shrink_to_fit();

    if ( img._operations.size() > 0 ) {
        std::shared_ptr<Operation> front = img._operations.front();
        if (front->get_type() == OperationType::READ) {
            start = 1;
            copy_cv(cv::imread(img._image_id, cv::IMREAD_ANYCOLOR));
        }
        else
            start = 0;

        for (int i = start; i < img._operations.size(); ++i)
            _operations.push_back(img._operations[i]);
    }

    delete temp;
}

ImageData::~ImageData()
{
    _operations.clear();
    _operations.shrink_to_fit();
    delete _tdb;
}


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string ImageData::get_image_id() const
{
    return _image_id;
}

ImageFormat ImageData::get_image_format() const
{
    return _format;
}

int ImageData::get_type() const
{
    return _cv_type;
}

cv::Size ImageData::get_dimensions()
{
    // TODO: iterate over operations themsevles to determine
    //          image size, rather than performing the operations
    if ( _operations.size() > 0 )
        perform_operations();
    return cv::Size(_width, _height);
}

long ImageData::get_size()
{
    if ( _height == 0 ) {
        if ( _format == VCL::TDB ) {
            if ( _tdb == NULL )
                throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                    stored in TDB format, but no data was found");
            return _tdb->get_image_size();
        }
        else {
            std::shared_ptr<Operation> op = _operations.front();
            (*op)(this);
            _operations.erase(_operations.begin());
        }
    }

    return long(_height) * long(_width) * _channels;
}

void ImageData::get_buffer(void* buffer, long buffer_size)
{
    perform_operations();

    switch ( _cv_type % 8 ) {
        case 0:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<unsigned char*>(buffer));
            else
                _tdb->get_buffer(static_cast<unsigned char*>(buffer), buffer_size);
            break;
        case 1:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<char*>(buffer));
            else
                _tdb->get_buffer(static_cast<char*>(buffer), buffer_size);
            break;
        case 2:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<unsigned short*>(buffer));
            else
                _tdb->get_buffer(static_cast<unsigned short*>(buffer), buffer_size);
            break;
        case 3:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<short*>(buffer));
            else
                _tdb->get_buffer(static_cast<short*>(buffer), buffer_size);
            break;
        case 4:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<int*>(buffer));
            else
                _tdb->get_buffer(static_cast<int*>(buffer), buffer_size);
            break;
        case 5:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<float*>(buffer));
            else
                _tdb->get_buffer(static_cast<float*>(buffer), buffer_size);
            break;
        case 6:
            if ( _format != VCL::TDB )
                copy_to_buffer(static_cast<double*>(buffer));
            else
                _tdb->get_buffer(static_cast<double*>(buffer), buffer_size);
            break;
        default:
            throw VCLException(UnsupportedFormat, _cv_type + " is not a \
                supported type");
            break;
    }
}


cv::Mat ImageData::get_cvmat()
{
    perform_operations();

    if ( _format != VCL::TDB )
        return _cv_img;
    else
        return _tdb->get_cvmat();
}

ImageData ImageData::get_area(const Rectangle &roi)
{
    ImageData area = *this;

    if ( area._format == VCL::TDB && area._operations.size() == 1 ) {
        if ( area._tdb == NULL )
            throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                stored in TDB format, but no data was found");
        area._operations.pop_back();
    }

    std::shared_ptr<Operation> op = std::make_shared<Crop> (roi, area._format);

    area._operations.push_back(op);

    area.perform_operations();

    area._height = roi.height;
    area._width = roi.width;

    return area;
}

std::vector<unsigned char> ImageData::get_encoded(ImageFormat format,
    const std::vector<int>& params)
{
    perform_operations();

    std::string extension = "." + format_to_string(format);

    if ( _cv_img.empty() ) {
        if ( _tdb == NULL)
            throw VCLException(ObjectEmpty, "No data to encode");
        else
            copy_cv(_tdb->get_cvmat());
    }

    std::vector<unsigned char> buffer;
    cv::imencode(extension, _cv_img, buffer, params);

    return buffer;
}


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
void ImageData::create_unique(const std::string &path,
    ImageFormat format)
{
    std::string unique_id;
    std::string name;

    std::string extension = format_to_string(format);

    const char& last = path.back();

    do {
        uint64_t id = get_uint64();
        std::stringstream ss;
        ss << std::hex << id;
        unique_id = ss.str();
        if (last != '/')
            name = path + "/" + unique_id + "." + extension;
        else
            name = path + unique_id + "." + extension;
    } while ( exists(name) );

    _image_id = name;
}

void ImageData::set_image_id(const std::string &image_id)
{
    _image_id = image_id;
}

void ImageData::set_format(const std::string &extension)
{
    if ( extension == "jpg" )
        _format = VCL::JPG;
    else if ( extension == "png" )
        _format = VCL::PNG;
    else if ( extension == "tdb" )
        _format = VCL::TDB;
    else
        throw VCLException(UnsupportedFormat, extension + " is not a \
            supported format");
}

void ImageData::set_type(int type)
{
    _cv_type = type;

    _channels = (type / 8) + 1;
}

void ImageData::set_compression(CompressionType comp)
{
    _compress = comp;
}

void ImageData::set_dimensions(cv::Size dimensions)
{
    _height = dimensions.height;
    _width = dimensions.width;

    if ( _format == VCL::TDB ) {
        if ( _tdb == NULL )
            throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                stored in TDB format, but no data was found");
        _tdb->set_image_properties(_height, _width, _channels);
    }
}

void ImageData::set_data_from_raw(void* buffer, long size)
{
    switch ( _cv_type % 8 ) {
        case 0:
            _tdb = new TDBImage(static_cast<unsigned char*>(buffer), size);
            break;
        case 1:
            _tdb = new TDBImage(static_cast<char*>(buffer), size);
            break;
        case 2:
            _tdb = new TDBImage(static_cast<unsigned short*>(buffer), size);
            break;
        case 3:
            _tdb = new TDBImage(static_cast<short*>(buffer), size);
            break;
        case 4:
            _tdb = new TDBImage(static_cast<int*>(buffer), size);
            break;
        case 5:
            _tdb = new TDBImage(static_cast<float*>(buffer), size);
            break;
        case 6:
            _tdb = new TDBImage(static_cast<double*>(buffer), size);
            break;
        default:
            throw VCLException(UnsupportedFormat, _cv_type + " is not a \
                supported type");
            break;
    }
}

void ImageData::set_data_from_encoded(const std::vector<unsigned char> &buffer)
{
    copy_cv(cv::imdecode(buffer, cv::IMREAD_ANYCOLOR));
}

void ImageData::set_minimum(int dimension)
{
    if ( _format == VCL::TDB ) {
        if ( _tdb == NULL )
            throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                stored in TDB format, but no data was found\n");
        _tdb->set_minimum(dimension);
    }
}


    /*  *********************** */
    /*   IMAGEDATA INTERACTION  */
    /*  *********************** */

void ImageData::perform_operations()
{
    for (int x = 0; x < _operations.size(); ++x) {
        std::shared_ptr<Operation> op = _operations[x];
        if ( op == NULL )
            throw VCLException(ObjectEmpty, "Nothing to be done");
        (*op)(this);
    }

    _operations.clear();
}

void ImageData::read(const std::string &image_id)
{
    _image_id = create_fullpath(image_id, _format);

    _operations.push_back(std::make_shared<Read> (_image_id, _format));
}

void ImageData::write(const std::string &image_id, ImageFormat img_format,
    bool metadata)
{
    _operations.push_back(std::make_shared<Write> (create_fullpath(image_id, img_format),
        img_format, _format, metadata));
}

void ImageData::resize(int rows, int columns)
{
    _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format));
}

void ImageData::crop(const Rectangle &rect)
{
    if ( _format == VCL::TDB && _operations.size() == 1 ) {
        if ( _tdb == NULL )
            throw VCLException(TileDBNotFound, "ImageFormat indicates image \
                stored in TDB format, but no data was found");
        _operations.pop_back();
    }

    _operations.push_back(std::make_shared<Crop> (rect, _format));
}

void ImageData::threshold(int value)
{
    _operations.push_back(std::make_shared<Threshold> (value, _format));
}

void ImageData::delete_object()
{
    if (_tdb != NULL)
        _tdb->delete_image();

    if (exists(_image_id)) {
        std::remove(_image_id.c_str());
    }
}

            /*  *********************** */
            /*         PRIVATE          */
            /*  *********************** */

    /*  *********************** */
    /*      COPY FUNCTIONS      */
    /*  *********************** */

void ImageData::copy_cv(const cv::Mat &cv_img)
{
    _channels = cv_img.channels();

    _height = cv_img.rows;
    _width = cv_img.cols;

    _cv_type = cv_img.type();

    _cv_img = cv_img.clone();
}

template <class T>
void ImageData::copy_to_buffer(T* buffer)
{
    int index = 0;

    int rows = _height;
    int columns = _width;

    if ( _cv_img.isContinuous() ) {
        columns *= rows;
        rows = 1;
    }

    for ( int i = 0; i < rows; ++i ) {
        for ( int j = 0; j < columns; ++j ) {
            if ( _channels == 1 )
                buffer[index] = T(_cv_img.at<unsigned char>(i, j));
            else {
                cv::Vec3b colors = _cv_img.at<cv::Vec3b>(i, j);
                for ( int x = 0; x < _channels; ++x ) {
                    buffer[index + x] = T(colors.val[x]);
                }
            }
            index += _channels;
        }
    }
}

template void ImageData::copy_to_buffer(unsigned char* buffer);
template void ImageData::copy_to_buffer(char* buffer);
template void ImageData::copy_to_buffer(unsigned short* buffer);
template void ImageData::copy_to_buffer(short* buffer);
template void ImageData::copy_to_buffer(int* buffer);
template void ImageData::copy_to_buffer(float* buffer);
template void ImageData::copy_to_buffer(double* buffer);

    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */
std::string ImageData::get_extension(const std::string &image_id)
{
    size_t file_ext = image_id.find_last_of(".");
    size_t dir_ext = image_id.find_last_of("/");

    if ( file_ext != std::string::npos ) {
        if ( file_ext > dir_ext + 2 )
            return image_id.substr(file_ext + 1);
        else
            throw VCLException(ObjectEmpty, image_id + " does not have a valid extension");
    }
    else
        return "";


}

std::string ImageData::format_to_string(ImageFormat image_format)
{
    switch( image_format )
    {
        case VCL::JPG:
            return "jpg";
        case VCL::PNG:
            return "png";
        case VCL::TDB:
            return "tdb";
        case VCL::NONE:
            return "";
        default:
            throw VCLException(UnsupportedFormat, image_format + " is not a \
                valid format");
    }
}

std::string ImageData::create_fullpath(const std::string &filename,
    ImageFormat format)
{
    if ( filename == "" )
        throw VCLException(ObjectNotFound, "Location to write object is undefined");

    std::string extension = get_extension(filename);
    std::string ext = format_to_string(format);

    if ( ext.compare(extension) == 0 || ext == "" )
        return filename;
    else
        return filename + "." + ext;
}

bool ImageData::exists(const std::string &name)
{
    struct stat filestatus;

    return (stat (name.c_str(), &filestatus) == 0);
}
