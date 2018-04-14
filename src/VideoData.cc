#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <iostream>
#include "VCL.h"
#include "VideoData.h"

using namespace VCL;

VideoData::Read::Read(const std::string& filename, VideoFormat format)
    : Operation(format),
      _fullpath(filename)
{
}

void VideoData::Read::operator()(VideoData *img)
{
    // if ( _format == VCL::TDB ) {
    //     if ( img->_tdb == NULL )
    //         throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //             stored in TDB format, but no data was found");

    //     img->_tdb->read();
    //     img->_height = img->_tdb->get_Video_height();
    //     img->_width = img->_tdb->get_Video_width();
    //     img->_channels = img->_tdb->get_Video_channels();
    // }
    // else {
    //     img->copy_cv(cv::imread(_fullpath, cv::IMREAD_ANYCOLOR));
    //     if ( img->_cv_img.empty() )
    //         throw VCLException(ObjectEmpty, _fullpath + " could not be read, \
    //             object is empty");
    // }
}

    /*  *********************** */
    /*       WRITE OPERATION    */
    /*  *********************** */
VideoData::Write::Write(const std::string& filename, VideoFormat format,
    VideoFormat old_format, bool metadata)
    : Operation(format),
      _old_format(old_format),
      _metadata(metadata),
      _fullpath(filename)
{
}

void VideoData::Write::operator()(VideoData *img)
{
    // if (_format == VCL::TDB) {
    //     if ( img->_tdb == NULL ) {
    //         img->_tdb = new TDBVideo(_fullpath);
    //         img->_tdb->set_compression(img->_compress);
    //     }

    //     if ( img->_tdb->has_data() )
    //         img->_tdb->write(_fullpath, _metadata);
    //     else
    //         img->_tdb->write(img->_cv_img, _metadata);
    // }
    // else {
    //     cv::Mat cv_img;
    //     if (_old_format == VCL::TDB)
    //         cv_img = img->_tdb->get_cvmat();
    //     else
    //         cv_img = img->_cv_img;

    //     if ( !cv_img.empty() )
    //         cv::imwrite(_fullpath, cv_img);
    //     else
    //         throw VCLException(ObjectEmpty, _fullpath + " could not be written \
    //             object is empty");
    // }
}

    /*  *********************** */
    /*       RESIZE OPERATION   */
    /*  *********************** */

void VideoData::Resize::operator()(VideoData *img)
{
    // if ( _format == VCL::TDB ) {
    //     img->_tdb->resize(_rect);
    //     img->_height = img->_tdb->get_Video_height();
    //     img->_width = img->_tdb->get_Video_width();
    //     img->_channels = img->_tdb->get_Video_channels();
    // }
    // else {
    //     if ( !img->_cv_img.empty() ) {
    //         cv::Mat cv_resized;
    //         cv::resize(img->_cv_img, cv_resized, cv::Size(_rect.width, _rect.height));
    //         img->copy_cv(cv_resized);
    //     }
    //     else
    //         throw VCLException(ObjectEmpty, "Video object is empty");
    // }
}

    /*  *********************** */
    /*       CROP OPERATION     */
    /*  *********************** */

void VideoData::Crop::operator()(VideoData *img)
{
    // if ( _format == VCL::TDB ) {
    //     img->_tdb->read(_rect);
    //     img->_height = img->_tdb->get_Video_height();
    //     img->_width = img->_tdb->get_Video_width();
    //     img->_channels = img->_tdb->get_Video_channels();
    // }
    // else {
    //     if ( !img->_cv_img.empty() ) {
    //         if ( img->_cv_img.rows < _rect.height + _rect.y || img->_cv_img.cols < _rect.width + _rect.x )
    //             throw VCLException(SizeMismatch, "Requested area is not within the Video");
    //         cv::Mat roi_img(img->_cv_img, _rect);
    //         img->copy_cv(roi_img);
    //     }
    //     else
    //         throw VCLException(ObjectEmpty, "Video object is empty");
    // }
}

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */

void VideoData::Threshold::operator()(VideoData *img)
{
    // if ( _format == VCL::TDB )
    //     img->_tdb->threshold(_threshold);
    // else {
    //     if ( !img->_cv_img.empty() )
    //         cv::threshold(img->_cv_img, img->_cv_img, _threshold, _threshold,
    //             cv::THRESH_TOZERO);
    //     else
    //         throw VCLException(ObjectEmpty, "Video object is empty");
    // }
}


                    /*  *********************** */
                    /*         VideoDATA        */
                    /*  *********************** */

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */

VideoData::VideoData()
{
    // _channels = 0;
    // _height = 0;
    // _width = 0;
    // _cv_type = CV_8UC3;

    // _format = VCL::NONE;
    // _compress = VCL::CompressionType::LZ4;

    // _tdb = NULL;
    // _Video_id = "";
}

VideoData::VideoData(const cv::Mat &cv_img)
{
    // copy_cv(cv_img);

    // _format = VCL::NONE;
    // _compress = VCL::CompressionType::LZ4;
    // _Video_id = "";

    // _tdb = NULL;
}

VideoData::VideoData(const std::string &Video_id)
{
    // _channels = 0;
    // _height = 0;
    // _width = 0;
    // _cv_type = CV_8UC3;

    // std::string extension = get_extension(Video_id);
    // set_format(extension);

    // _compress = VCL::CompressionType::LZ4;

    // _Video_id = create_fullpath(Video_id, _format);

    // if ( _format == VCL::TDB ) {
    //     _tdb = new TDBVideo(_Video_id);
    //     _tdb->set_compression(_compress);
    // }
    // else
    //     _tdb = NULL;

}

// VideoData::VideoData(void* buffer, cv::Size dimensions, int cv_type)
// {
//     // _height = dimensions.height;
//     // _width = dimensions.width;
//     // _cv_type = cv_type;
//     // _channels = (cv_type / 8) + 1;

//     // _format = VCL::TDB;
//     // _compress = VCL::CompressionType::LZ4;
//     // _Video_id = "";

//     // set_data_from_raw(buffer, _height*_width*_channels);
//     // _tdb->set_compression(_compress);
// }

VideoData::VideoData(const VideoData &img)
{
    // _format = img._format;
    // _compress = img._compress;
    // _Video_id = img._Video_id;

    // if ( !(img._cv_img).empty() )
    //     copy_cv(img._cv_img);

    // if ( img._tdb != NULL )
    //     _tdb = new TDBVideo(*img._tdb);
    // else
    //     _tdb = NULL;

    // int start;
    // if ( img._operations.size() > 0 ) {
    //     std::shared_ptr<Operation> front = img._operations.front();
    //     if (front->get_type() == OperationType::READ) {
    //         start = 1;
    //         copy_cv(cv::imread(img._Video_id, cv::IMREAD_ANYCOLOR));
    //     }
    //     else
    //         start = 0;

    //     for (int i = start; i < img._operations.size(); ++i)
    //         _operations.push_back(img._operations[i]);
    // }
}

void VideoData::operator=(const VideoData &img)
{
    // TDBVideo *temp = _tdb;

    // if ( !(img._cv_img).empty() )
    //     copy_cv(img._cv_img);
    // else {
    //     _channels = img._channels;

    //     _height = img._height;
    //     _width = img._width;

    //     _cv_type = img._cv_type;
    // }

    // _format = img._format;
    // _compress = img._compress;
    // _Video_id = img._Video_id;

    // if ( img._tdb != NULL ) {
    //     _tdb = new TDBVideo(*img._tdb);
    // }
    // else
    //     _tdb = NULL;

    // int start;

    // _operations.clear();
    // _operations.shrink_to_fit();

    // if ( img._operations.size() > 0 ) {
    //     std::shared_ptr<Operation> front = img._operations.front();
    //     if (front->get_type() == OperationType::READ) {
    //         start = 1;
    //         copy_cv(cv::imread(img._Video_id, cv::IMREAD_ANYCOLOR));
    //     }
    //     else
    //         start = 0;

    //     for (int i = start; i < img._operations.size(); ++i)
    //         _operations.push_back(img._operations[i]);
    // }

    // delete temp;
}

VideoData::~VideoData()
{
    // _operations.clear();
    // _operations.shrink_to_fit();
    // delete _tdb;
}


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string VideoData::get_Video_id() const
{
    return _Video_id;
}

VideoFormat VideoData::get_Video_format() const
{
    return _format;
}

int VideoData::get_type() const
{
   // return _cv_type;
    return 0;
}

cv::Size VideoData::get_dimensions()
{
    // TODO: iterate over operations themsevles to determine
    //          Video size, rather than performing the operations
    // if ( _operations.size() > 0 )
    //     perform_operations();
    return cv::Size(10, 10);
}

int VideoData::get_size()
{
    // if ( _height == 0 ) {
    //     if ( _format == VCL::TDB ) {
    //         if ( _tdb == NULL )
    //             throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //                 stored in TDB format, but no data was found");
    //         return _tdb->get_Video_size();
    //     }
    //     else {
    //         std::shared_ptr<Operation> op = _operations.front();
    //         (*op)(this);
    //         _operations.erase(_operations.begin());
    //     }
    // }

    // return int(_height) * int(_width) * _channels;
    return 1;
}

void VideoData::get_buffer(void* buffer, int buffer_size)
{
    // perform_operations();

    // switch ( _cv_type % 8 ) {
    //     case 0:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<unsigned char*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<unsigned char*>(buffer), buffer_size);
    //         break;
    //     case 1:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<char*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<char*>(buffer), buffer_size);
    //         break;
    //     case 2:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<unsigned short*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<unsigned short*>(buffer), buffer_size);
    //         break;
    //     case 3:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<short*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<short*>(buffer), buffer_size);
    //         break;
    //     case 4:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<int*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<int*>(buffer), buffer_size);
    //         break;
    //     case 5:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<float*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<float*>(buffer), buffer_size);
    //         break;
    //     case 6:
    //         if ( _format != VCL::TDB )
    //             copy_to_buffer(static_cast<double*>(buffer));
    //         else
    //             _tdb->get_buffer(static_cast<double*>(buffer), buffer_size);
    //         break;
    //     default:
    //         throw VCLException(UnsupportedFormat, _cv_type + " is not a \
    //             supported type");
    //         break;
    // }
}


cv::Mat VideoData::get_cvmat()
{
    // perform_operations();

    // if ( _format != VCL::TDB )
    //     return _cv_img;
    // else
    //     return _tdb->get_cvmat();
}

VideoData VideoData::get_area(const Rectangle &roi)
{
    VideoData area = *this;

    // if ( area._format == VCL::TDB && area._operations.size() == 1 ) {
    //     if ( area._tdb == NULL )
    //         throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //             stored in TDB format, but no data was found");
    //     area._operations.pop_back();
    // }

    // std::shared_ptr<Operation> op = std::make_shared<Crop> (roi, area._format);

    // area._operations.push_back(op);

    // area.perform_operations();

    // area._height = roi.height;
    // area._width = roi.width;

     return area;
}

std::vector<unsigned char> VideoData::get_encoded(VideoFormat format,
    const std::vector<int>& params)
{
    // perform_operations();

    // std::string extension = "." + format_to_string(format);

    // if ( _cv_img.empty() ) {
    //     if ( _tdb == NULL)
    //         throw VCLException(ObjectEmpty, "No data to encode");
    //     else
    //         copy_cv(_tdb->get_cvmat());
    // }

    // std::vector<unsigned char> buffer;
    // cv::imencode(extension, _cv_img, buffer, params);

    // return buffer;
}


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
void VideoData::create_unique(const std::string &path,
    VideoFormat format)
{
    // std::string unique_id;
    // std::string name;

    // std::string extension = format_to_string(format);

    // const char& last = path.back();

    // do {
    //     uint64_t id = get_int64();
    //     std::stringstream ss;
    //     ss << std::hex << id;
    //     unique_id = ss.str();
    //     if (last != '/')
    //         name = path + "/" + unique_id + "." + extension;
    //     else
    //         name = path + unique_id + "." + extension;
    // } while ( exists(name) );

    // _Video_id = name;
}

void VideoData::set_Video_id(const std::string &Video_id)
{
    // _Video_id = Video_id;
}

void VideoData::set_format(const std::string &extension)
{
    // if ( extension == "jpg" )
    //     _format = VCL::JPG;
    // else if ( extension == "png" )
    //     _format = VCL::PNG;
    // else if ( extension == "tdb" )
    //     _format = VCL::TDB;
    // else
    //     throw VCLException(UnsupportedFormat, extension + " is not a \
    //         supported format");
}

void VideoData::set_type(int type)
{
    // _cv_type = type;

    // _channels = (type / 8) + 1;
}



void VideoData::set_dimensions(cv::Size dimensions)
{
    // _height = dimensions.height;
    // _width = dimensions.width;

    // if ( _format == VCL::TDB ) {
    //     if ( _tdb == NULL )
    //         throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //             stored in TDB format, but no data was found");
    //     _tdb->set_Video_properties(_height, _width, _channels);
    // }
}

void VideoData::set_data_from_raw(void* buffer, int size)
{
    // switch ( _cv_type % 8 ) {
    //     case 0:
    //         _tdb = new TDBVideo(static_cast<unsigned char*>(buffer), size);
    //         break;
    //     case 1:
    //         _tdb = new TDBVideo(static_cast<char*>(buffer), size);
    //         break;
    //     case 2:
    //         _tdb = new TDBVideo(static_cast<unsigned short*>(buffer), size);
    //         break;
    //     case 3:
    //         _tdb = new TDBVideo(static_cast<short*>(buffer), size);
    //         break;
    //     case 4:
    //         _tdb = new TDBVideo(static_cast<int*>(buffer), size);
    //         break;
    //     case 5:
    //         _tdb = new TDBVideo(static_cast<float*>(buffer), size);
    //         break;
    //     case 6:
    //         _tdb = new TDBVideo(static_cast<double*>(buffer), size);
    //         break;
    //     default:
    //         throw VCLException(UnsupportedFormat, _cv_type + " is not a \
    //             supported type");
    //         break;
    // }
}

void VideoData::set_data_from_encoded(const std::vector<unsigned char> &buffer)
{
    // copy_cv(cv::imdecode(buffer, cv::IMREAD_ANYCOLOR));
}

void VideoData::set_minimum(int dimension)
{
    // if ( _format == VCL::TDB ) {
    //     if ( _tdb == NULL )
    //         throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //             stored in TDB format, but no data was found\n");
    //     _tdb->set_minimum(dimension);
    // }
}


    /*  *********************** */
    /*   VideoDATA INTERACTION  */
    /*  *********************** */

void VideoData::perform_operations()
{
    // for (int x = 0; x < _operations.size(); ++x) {
    //     std::shared_ptr<Operation> op = _operations[x];
    //     if ( op == NULL )
    //         throw VCLException(ObjectEmpty, "Nothing to be done");
    //     (*op)(this);
    // }

    // _operations.clear();
}

void VideoData::read(const std::string &Video_id)
{
    // _Video_id = create_fullpath(Video_id, _format);

    // _operations.push_back(std::make_shared<Read> (_Video_id, _format));
}

void VideoData::write(const std::string &Video_id, VideoFormat img_format,
    bool metadata)
{
    // _operations.push_back(std::make_shared<Write> (create_fullpath(Video_id, img_format),
    //     img_format, _format, metadata));
}

void VideoData::resize(int rows, int columns)
{
    // _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format));
}

void VideoData::crop(const Rectangle &rect)
{
    // if ( _format == VCL::TDB && _operations.size() == 1 ) {
    //     if ( _tdb == NULL )
    //         throw VCLException(TileDBNotFound, "VideoFormat indicates Video \
    //             stored in TDB format, but no data was found");
    //     _operations.pop_back();
    // }

    // _operations.push_back(std::make_shared<Crop> (rect, _format));
}

void VideoData::threshold(int value)
{
   // _operations.push_back(std::make_shared<Threshold> (value, _format));
}

void VideoData::delete_object()
{
    // if (_tdb != NULL)
    //     _tdb->delete_Video();

    // if (exists(_Video_id)) {
    //     std::remove(_Video_id.c_str());
    // }
}

            /*  *********************** */
            /*         PRIVATE          */
            /*  *********************** */

    /*  *********************** */
    /*      COPY FUNCTIONS      */
    /*  *********************** */

void VideoData::copy_cv(const cv::Mat &cv_img)
{
    // _channels = cv_img.channels();

    // _height = cv_img.rows;
    // _width = cv_img.cols;

    // _cv_type = cv_img.type();

    // _cv_img = cv_img.clone();
}

template <class T>
void VideoData::copy_to_buffer(T* buffer)
{
    // int index = 0;

    // int rows = _height;
    // int columns = _width;

    // if ( _cv_img.isContinuous() ) {
    //     columns *= rows;
    //     rows = 1;
    // }

    // for ( int i = 0; i < rows; ++i ) {
    //     for ( int j = 0; j < columns; ++j ) {
    //         if ( _channels == 1 )
    //             buffer[index] = T(_cv_img.at<unsigned char>(i, j));
    //         else {
    //             cv::Vec3b colors = _cv_img.at<cv::Vec3b>(i, j);
    //             for ( int x = 0; x < _channels; ++x ) {
    //                 buffer[index + x] = T(colors.val[x]);
    //             }
    //         }
    //         index += _channels;
    //     }
    // }
}



    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */
std::string VideoData::get_extension(const std::string &Video_id)
{
    // size_t file_ext = Video_id.find_last_of(".");
    // size_t dir_ext = Video_id.find_last_of("/");

    // if ( file_ext != std::string::npos ) {
    //     if ( file_ext > dir_ext + 2 )
    //         return Video_id.substr(file_ext + 1);
    //     else
    //         throw VCLException(ObjectEmpty, Video_id + " does not have a valid extension");
    // }
    // else
    //     return "";
    return "";


}

std::string VideoData::format_to_string(VideoFormat Video_format)
{
    // switch( Video_format )
    // {
    //     case VCL::JPG:
    //         return "jpg";
    //     case VCL::PNG:
    //         return "png";
    //     case VCL::TDB:
    //         return "tdb";
    //     case VCL::NONE:
    //         return "";
    //     default:
    //         throw VCLException(UnsupportedFormat, Video_format + " is not a \
    //             valid format");
    // }
    return " Format";
}

std::string VideoData::create_fullpath(const std::string &filename,
    VideoFormat format)
{
    // if ( filename == "" )
    //     throw VCLException(ObjectNotFound, "Location to write object is undefined");

    // std::string extension = get_extension(filename);
    // std::string ext = format_to_string(format);

    // if ( ext.compare(extension) == 0 || ext == "" )
    //     return filename;
    // else
    //     return filename + "." + ext;
    return "id";
}

bool VideoData::exists(const std::string &name)
{
    // struct stat filestatus;

    // return (stat (name.c_str(), &filestatus) == 0);
    return true;
}
