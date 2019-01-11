#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include "VCL.h"
using namespace std;

#include "VideoData.h"

using namespace VCL;

void VideoData::Operation::set_fourcc()
{
    switch( _format )
    {
        case VCL::Video::Format::MP4:
            _fourcc = CV_FOURCC('H', '2', '6', '+');
            break;
        case VCL::Video::Format::AVI:
            _fourcc = CV_FOURCC('X', 'V', 'I', 'D');
            break;
        case VCL::Video::Format::MPEG:
            _fourcc = CV_FOURCC('M', 'P', 'E', 'G');
            // _fourcc[0] = 'M';
            // _fourcc[1] = 'P';
            // _fourcc[2] = 'E';
            // _fourcc[3] = 'G';
            // break;
        case VCL::Video::Format::XVID:
            _fourcc = CV_FOURCC('X', 'V', 'I', 'D');
            break;
        default:
            throw VCLException(UnsupportedFormat, (int)_format + " is not a \
                valid format");
    }
}


VideoData::Read::Read(const std::string& filename, VCL::Video::Format format)
    : Operation(format),
      _fullpath(filename)
{
}

void VideoData::Read::operator()(VideoData *video)
{
}
    /*  *********************** */
    /*       WRITE OPERATION    */
    /*  *********************** */
VideoData::Write::Write(const std::string& filename, VCL::Video::Format format,
    VCL::Video::Format old_format)
    : Operation(format),
      _old_format(old_format),
      _fullpath(filename)
{

}

void VideoData::Write::operator()(VideoData *video)
{
    set_fourcc();
    std::cout<< "inside write \t"<< video->_fullpath<< std::endl;
    video->_outputVideo = cv::VideoWriter(video->_fullpath,
                    _fourcc,
                    video->_fps,
                    cv::Size(video->_frame_width, video->_frame_height));
    int count=video->_start_frame;
    while (count < video->_end_frame)
    {
        cv::Mat frame;
        video->_cv_video >> frame;
        if ( !frame.empty() ) {
            video->_outputVideo.write(frame);
        }
        else
            throw VCLException(ObjectEmpty, "Frame is empty");

        count+=video->_step;
  }


  video->_cv_video = cv::VideoCapture(video->_fullpath);
  video->_cv_video.release();
}

    /*  *********************** */
    /*       RESIZE OPERATION   */
    /*  *********************** */

void VideoData::Resize::operator()(VideoData *video)
{
    set_fourcc();
    video->_outputVideo = cv::VideoWriter(video->_fullpath,
                    _fourcc,
                    video->_fps,
                    cv::Size(_rect.width, _rect.height));

    video->_cv_video.set(CV_CAP_PROP_POS_FRAMES, video->_start_frame - 1);

    int count = video->_start_frame;
      while ( count < video->_end_frame ) {
        cv::Mat frame;
        video->_cv_video >> frame;
        if ( !frame.empty() ) {
             cv::Mat cv_resized;
             cv::resize(frame, cv_resized, cv::Size(_rect.width, _rect.height));
            // cv::Mat roi_frame(frame, cv_resized);
            video->_outputVideo.write(cv_resized);
        }
        else
            throw VCLException(ObjectEmpty, "Frame is empty");

        count+=video->_step;
    }

    video->_temp_exist = true;
    video->_cv_video = cv::VideoCapture(video->_fullpath);
    video->_cv_video.release();
}

    /*  *********************** */
    /*       CROP OPERATION     */
    /*  *********************** */

void VideoData::Crop::operator()(VideoData *video)
{
    set_fourcc();
    video->_outputVideo = cv::VideoWriter(video->_fullpath,
                    _fourcc,
                    video->_fps,
                    cv::Size(_rect.width, _rect.height));
    //video->_cv_video.set(CV_CAP_PROP_POS_FRAMES, video->_start_frame - 1);
    int count = video->_start_frame;
    while ( count < video->_end_frame ) {
        cv::Mat frame;
        if ( video->_cv_video.read(frame) ) {
            if ( !frame.empty() ) {
                if ( frame.rows < _rect.height + _rect.y || frame.cols < _rect.width + _rect.x )
                    throw VCLException(SizeMismatch, "Requested area is not within the Video");
                cv::Mat roi_frame(frame, _rect);
                video->_outputVideo.write(roi_frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=video->_step;
    }
    video->_temp_exist = true;
    video->_cv_video = cv::VideoCapture(video->_fullpath);
    video->_cv_video.release();
}

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */

void VideoData::Threshold::operator()(VideoData *video)
{
    set_fourcc();
    video->_outputVideo = cv::VideoWriter(video->_fullpath,
                    _fourcc,
                    video->_fps,
                    cv::Size(video->_frame_width, video->_frame_height));

    // video->_cv_video.set(CV_CAP_PROP_POS_FRAMES, video->_start_frame - 1);
    int count = video->_start_frame;
    while ( count < video->_end_frame ) {
        cv::Mat frame;
        if ( video->_cv_video.read(frame) ) {
            if ( !frame.empty() ) {
                 cv::threshold(frame, frame, _threshold, _threshold,
                 cv::THRESH_TOZERO);
                 video->_outputVideo.write(frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=video->_step;

    }

    video->_temp_exist = true;
    video->_cv_video = cv::VideoCapture(video->_fullpath);
    video->_cv_video.release();
}
                    /*  *********************** */
                    /*      Interval Operation  */
                    /*  *********************** */
void VideoData::Interval::operator()(VideoData *video)
{

}

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
void VideoData::set_cv_video(cv::VideoCapture& v)
{
    _cv_video=v;

    _temporary_path = "videos_test/";

    _start_frame =0;
    _fps = static_cast<float>(v.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<long>(v.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<long>(v.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<long>(v.get(CV_CAP_PROP_FRAME_HEIGHT));
    _video_time = _frame_count/_fps;
    _length = _frame_count;
    _end_frame = _frame_count;

    if(!v.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open VideoCapture");
    }

    int ex = static_cast<int>(v.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT);
    _fullpath = create_unique(_temporary_path, _format);
}

VideoData::VideoData()
{
    _length=0;
    _start_frame =0;
    _end_frame =0;
    _frame_count =0;
    _fps=0;
    _frame_width=0;
    _frame_height=0;
    _video_id ="";
    _format = VCL::Video::Format::AVI;
    _temporary_path ="videos_test/";
    _fullpath = create_unique(_temporary_path, _format);
}


VideoData::VideoData(const VideoData &video)
{
    copy_cv(_cv_video); // TODO
    _format = video._format;
    _compress = VCL::Video::CompressionType::NOCOMPRESSION_Video ;
    _temporary_path ="videos_test/";
    _fullpath = create_unique(_temporary_path, _format);
    _video_id = "";
}

VideoData::VideoData( const std::string &video_id )
{
    _video_id = video_id;
    _cv_video =  cv::VideoCapture(video_id);
    _temporary_path = "videos_test/";

    _start_frame =0;
    _fps = static_cast<float>(_cv_video.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<long>(_cv_video.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<long>(_cv_video.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<long>(_cv_video.get(CV_CAP_PROP_FRAME_HEIGHT));
    _video_time = _frame_count/_fps;
    _length = _frame_count;
    _end_frame = _frame_count;

    if(!_cv_video.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + video_id);
    }

    int ex = static_cast<int>(_cv_video.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT);
    _fullpath = create_unique(_temporary_path, _format);
}

VideoData::VideoData(void* buffer, long size)
{
    std::fstream outfile;
    _temporary_path = "videos_test/";
    _fullpath = create_unique(_temporary_path, VCL::Video::Format::AVI);
    outfile.open(_fullpath, std::ios::out);
    if (outfile.is_open())
    {
        outfile.write(reinterpret_cast<char*>(buffer), size);
        outfile.close();
        _temp_exist = true; // to check the path
    }
    else std::cout<< " The Video Blob was not opened!"<<std::endl;

 //after we recieved the video file, we create a VideoCapture out of it in-order toget the vide details.
    _cv_video =  cv::VideoCapture(_fullpath);
    _fps         = static_cast<float>(_cv_video.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_cv_video.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_cv_video.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_cv_video.get(CV_CAP_PROP_FRAME_HEIGHT));
    _video_time = _frame_count/_fps;
    _length = _frame_count;
    _end_frame = _frame_count;
    _start_frame =0;

    if(!_cv_video.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the Video File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " +_fullpath);
    }
    int ex = static_cast<int>(_cv_video.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT); // to get the current extention of the video file
}

VideoData::VideoData(const cv::VideoCapture &cv_video )
{
    _cv_video = cv_video;
    if(!_cv_video.isOpened())  // check if we succeeded
        std::cout<<" Error in Opening the File " <<std::endl;
    else {
        _frame_count = (int) _cv_video.get(CV_CAP_PROP_FRAME_COUNT);
        _fps = static_cast<float>(_cv_video.get(CV_CAP_PROP_FPS));
        int ex = static_cast<int>(_cv_video.get(CV_CAP_PROP_FOURCC));
        char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
        set_format_from_extension(EXT);
        _frame_width = _cv_video.get(CV_CAP_PROP_FRAME_WIDTH);
        _frame_height = _cv_video.get(CV_CAP_PROP_FRAME_HEIGHT);
        _end_frame = _frame_count;
        _start_frame =0;
    }

    _temporary_path ="videos_test/";
    _fullpath = create_unique(_temporary_path, _format);
}

cv::VideoWriter VideoData::get_output_video(){
    return _outputVideo;
}

std::string VideoData::get_fullpath(void){
    return _fullpath;
}
void VideoData::read(const std::string &video_id){

   _operations.push_back(std::make_shared<Read> (_video_id, _format));
}

void VideoData::write( const std::string &video_id, VCL::Video::Format video_format
           )
{
    _fullpath = create_fullpath(video_id, video_format);
    _operations.push_back(std::make_shared<Write> (_fullpath,
        video_format, _format));
}

void VideoData::operator=(const VideoData &video)
{
  _video->_video_id = video.get_video_id();
  _video->_fps =  video.get_frame_count();
}

VideoData::~VideoData()
{
    _cv_video.release();
    _outputVideo.release();
}

    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string VideoData::get_video_id() const
{
    return _fullpath;
}
cv::VideoCapture VideoData::get_cv_video() const
{
    return _cv_video;
}

VCL::Video::Format VideoData::get_video_format() const
{
    return _format;
}

long VideoData::get_frame_count() const
{
    // return _cv_video.get(CV_CAP_PROP_FRAME_COUNT);
    return _frame_count;
}

cv::Size VideoData::get_dimensions()
{
     // _dim = cv::Size((int) _cv_video.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
     //             (int) _cv_video.get(CV_CAP_PROP_FRAME_HEIGHT));
    _dim = cv::Size(_frame_width, _frame_height);
    return _dim;
}

VCL::Video::Video_Size VideoData::get_size()
{
    // _size.width = (long) _cv_video.get(CV_CAP_PROP_FRAME_WIDTH);    // Acquire input size
    // _size.height=  (long) _cv_video.get(CV_CAP_PROP_FRAME_HEIGHT);
    // _size.frame_number = (long) _cv_video.get(CV_CAP_PROP_FRAME_COUNT);
    _size.width = _frame_width;
    _size.height = _frame_height;
    _size.frame_number = _frame_count;
    return _size;
}

void VideoData::get_buffer(void* buffer, int buffer_size)
{
   perform_operations();
}

VideoData VideoData::get_area(const Rectangle &roi)
{
    VideoData area = *this;
    return area;
}

char* VideoData::get_encoded(VCL::Video::Format format,
    const std::vector<int>& params)
{
    perform_operations();
    std::string extension = "." + format_to_string(format);
    std::ifstream ifile;
    ifile.open(_fullpath, std::ifstream::in);
    char* inBuf;
    ifile.seekg(0, std::ios::end);
    _encoded_size = (long)ifile.tellg();
    ifile.seekg(0, std::ios::beg);
    inBuf = new char[_encoded_size];
    ifile.read(inBuf, _encoded_size);
    // std::string json_query = std::string(inBuf);
    ifile.close();
    return inBuf;
}

long VideoData::get_size_encoded()
{
    return _encoded_size;
}

    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
std::string VideoData::remove_extention(const std::string old_name)
{
    _video_id = VCL::remove_extention(old_name);
    return _video_id;
}

std::string VideoData::create_unique(const std::string &path, VCL::Video::Format format)
{
    std::string extension = format_to_string(format);
    std::string name = create_unique_path(path, extension);
    _fullpath = name;

    return name;
}

std::string VideoData::format_to_string(VCL::Video::Format format)
{
    switch( format )
    {
        case VCL::Video::Format::MP4:
            return "mp4";
        case VCL::Video::Format::AVI:
            return "avi";
        case VCL::Video::Format::MPEG:
            return "mpeg";
        case VCL::Video::Format::XVID:
            return "xvid";
        default:
            throw VCLException(UnsupportedFormat, (int)format + " is not a \
                valid format");
    }
}

void VideoData::set_video_id(const std::string &video_id)
{
  _video_id = video_id;
}

void VideoData::set_format_from_extension(const std::string &extension)
{
    std::string extent;
    for ( auto elem : extension ) {
        if (std::isalpha(elem))
            extent += std::tolower(elem);
        else
            extent += elem;
    }
    if ( extent == "avi" )
        _format = VCL::Video::Format::AVI;
    else if ( extent == "mp4" || extent == "fmp4" )
        _format = VCL::Video::Format::MP4;
    else if ( extent == "mpeg" )
        _format = VCL::Video::Format::MPEG;
    else if ( extent == "xvid" )
        _format = VCL::Video::Format::XVID;
    else
        throw VCLException(UnsupportedFormat, extension + " is not a " +
            "supported format");
}

void VideoData::set_format(int form)
{
    switch (form) {
        case 4:
            _format = VCL::Video::Format::MP4;
            break;
        case 5:
            _format = VCL::Video::Format::AVI;
            break;
        case 6:
            _format = VCL::Video::Format::MPEG;
            break;
        default:
            throw VCLException(UnsupportedFormat, form + " is not a supported video format");
    }
}

void VideoData::set_temporary_directory(const std::string &path)
{
    _temporary_path = path;
}
void VideoData::set_dimensions(cv::Size dimensions)
{
    _frame_height = dimensions.height;
    _frame_width = dimensions.width;

}
void VideoData::set_compression(Video::CompressionType comp)
{
    _compress = comp;
}

    /*  *********************** */
    /*   VideoDATA INTERACTION  */
    /*  *********************** */

void VideoData::perform_operations()
{
    bool written = false;

    for (int x = 0; x < _operations.size(); ++x) {
        std::shared_ptr<Operation> op = _operations[x];
        if ( op == NULL )
            throw VCLException(ObjectEmpty, "Nothing to be done");
        if (written)
            continue;
        else {
            if ( op->get_type() == OperationType::RESIZE || op->get_type() == OperationType::CROP || op->get_type() == OperationType::THRESHOLD )
                written = true;
            (*op)(this);
        }
    }
    _operations.clear();
}

void VideoData::resize(int rows, int columns)
{
    _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format));
}

void VideoData::interval(VCL::Video::UNIT u, int start, int stop, int step)
{
    _video_unit=u;
    _start_frame = start;
    _end_frame = stop;
    _step = step;
    _frame_count = (stop - start)/step + 1;
}

void VideoData::crop(const Rectangle &rect)
{
    _operations.push_back(std::make_shared<Crop> (rect, _format));
}

void VideoData::threshold(int value)
{
    _operations.push_back(std::make_shared<Threshold> (value, _format));
}
void VideoData::delete_object()
{
 if (VCL::exists(_video_id)) {
        std::remove(_video_id.c_str());
    }

}
    /*  *********************** */
    /*      COPY FUNCTIONS      */
    /*  *********************** */

void VideoData::copy_cv(const cv::VideoCapture &video_id)
{
    _cv_video =  video_id;
    _fps = _cv_video.get(CV_CAP_PROP_FPS);
    _frame_count= _cv_video.get(CV_CAP_PROP_FRAME_COUNT);
    _frame_width = _cv_video.get(CV_CAP_PROP_FRAME_WIDTH);
    _frame_height = _cv_video.get(CV_CAP_PROP_FRAME_HEIGHT);

}
    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */

std::string VideoData::create_fullpath(const std::string &filename,
    VCL::Video::Format format)
{
    if ( filename == "" )
        throw VCLException(ObjectNotFound, "Location to write object is undefined");
    std::string extension = VCL::get_extension(filename);
    std::string ext = format_to_string(format);
    if ( ext.compare(extension) == 0 || ext == "" )
        return filename;
    else
        return filename + "." + ext;
}
