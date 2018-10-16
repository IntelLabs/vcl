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

VideoData::Read::Read(const std::string& filename, Format format)
    : Operation(format),
      _fullpath(filename)
{
}

void VideoData::Read::operator()(VideoData *video)
{
    _start = video->_start_frame;
    _stop = video->_end_frame;
    _step = video->_step;

    video->_inputVideo.set(CV_CAP_PROP_POS_FRAMES, _start - 1);
    int count = _start;

    while ( count < _stop ) {
        cv::Mat frame;
        if ( video->_inputVideo.read(frame) ) {
            if ( !frame.empty() ) {
               // cv::imshow( "Frame", frame );
                // Press  ESC on keyboard to  exit
                 char c = (char)cv::waitKey(1);
                 if( c == 27 )
                 break;
           }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }
    video->_temp_exist = true;

        if ( !(video->_inputVideo.isOpened()) )
            throw VCLException(ObjectEmpty, _fullpath + " could not be read, \
                object is empty");

}

    /*  *********************** */
    /*       WRITE OPERATION    */
    /*  *********************** */
VideoData::Write::Write(const std::string& filename, Format format,
    Format old_format, bool metadata)
    : Operation(format),
      _old_format(old_format),
      _metadata(metadata),
      _fullpath(filename)
{
}

void VideoData::Write::operator()(VideoData *video)
{
    _start = video->_start_frame;
    _stop = video->_end_frame;
    _step = video->_step;
    cv::VideoWriter _outputVideo(_fullpath,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    video->_fps,
                    cv::Size(video->_frame_width, video->_frame_height));
    int count=_start;

    while (count <=_stop)
    {
        cv::Mat frame;

        // Capture frame-by-frame
        video->_inputVideo >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break; // probably want to throw if the frame is empty

        // Write the frame into the file 'outcpp.avi'
         _outputVideo.write(frame);

        // Display the resulting frame
        //cv::imshow( "Frame", frame );

        // Press  ESC on keyboard to  exit
        // char c = (char)cv::waitKey(1);
        // if( c == 27 )
        //     break;
        count+=_step;
  }

}

    /*  *********************** */
    /*       RESIZE OPERATION   */
    /*  *********************** */

void VideoData::Resize::operator()(VideoData *video)
{
    _start = video->_start_frame;
    _stop = video->_end_frame;
    _step = video->_step;

    video->create_unique(video->_temporary_path, VCL::Format::AVI);
    std::string cropped_vid = video->remove_extention(video->_video_id) + "resized.avi" ;
    std::cout << cropped_vid << std::endl;
    video->_outputVideo = cv::VideoWriter(cropped_vid,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    video->_fps,
                    cv::Size(_rect.width, _rect.height));

    video->_inputVideo.set(CV_CAP_PROP_POS_FRAMES, _start - 1);
    int count = _start;
      while ( count < _stop ) {
        cv::Mat frame;
        if ( video->_inputVideo.read(frame) ) {
            if ( !frame.empty() ) {
                 cv::Mat cv_resized;
                 cv::resize(frame, cv_resized, cv::Size(_rect.width, _rect.height));
                // cv::Mat roi_frame(frame, cv_resized);
                video->_outputVideo.write(cv_resized);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }

    video->_temp_exist = true;
}




    /*  *********************** */
    /*       CROP OPERATION     */
    /*  *********************** */

void VideoData::Crop::operator()(VideoData *video)
{
    _start = video->_start_frame;
    _stop = video->_end_frame;
    _step = video->_step;

   std::cout<<video->_temporary_path<<std::endl;
    video->create_unique(video->_temporary_path, VCL::Format::AVI);

    std::string cropped_vid = video->remove_extention(video->_video_id) + "_cropped.avi" ;
    video->_outputVideo = cv::VideoWriter(cropped_vid,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    video->_fps,
                    cv::Size(_rect.width, _rect.height));

    video->_inputVideo.set(CV_CAP_PROP_POS_FRAMES, _start - 1);
    int count = _start;

    while ( count < _stop ) {
        cv::Mat frame;
        if ( video->_inputVideo.read(frame) ) {
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
        count+=_step;
    }

    video->_temp_exist = true;
}

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */

void VideoData::Threshold::operator()(VideoData *video)
{
    _start = video->_start_frame;
    _stop = video->_end_frame;
    _step = video->_step;

    video->create_unique(video->_temporary_path, VCL::Format::AVI);
    std::string cropped_vid = video->remove_extention(video->_video_id) + "_threshold.avi" ;
    std::cout << cropped_vid << std::endl;
    video->_outputVideo = cv::VideoWriter(cropped_vid,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    video->_fps,
                    cv::Size(video->_frame_width, video->_frame_height));

    video->_inputVideo.set(CV_CAP_PROP_POS_FRAMES, _start - 1);
    int count = _start;

    while ( count < _stop ) {
        cv::Mat frame;
        if ( video->_inputVideo.read(frame) ) {
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
        count+=_step;
    }

    video->_temp_exist = true;
}
                    /*  *********************** */
                    /*      Interval Operation  */
                    /*  *********************** */

void VideoData::Interval::operator()(VideoData *video)
{

}


                    /*  *********************** */
                    /*         VideoDATA        */
                    /*  *********************** */

    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */

VideoData::VideoData()
{

    std::cout<< "Hello Empty"<<std::endl;
    _width = 0;
    _height =0;
    _length=0;
    _start_frame =0;
    _end_frame =0;
    _frame_count =0;
    _fps=0;
    _frame_width=0;
    _frame_height=0;
    _video_id ="";
    _format = VCL::Format::NONE;
    _temporary_path ="/home/ragaad/vdms-video/tests/db/videos";
;
}


VideoData::VideoData(const VideoData &video)
{
     copy_cv(_inputVideo); // TODO

     _format = video._format;
    // _compress = VCL::CompressionType::LZ4;
     _video_id = "";


}

VideoData::VideoData( const std::string &video_id )
{

    //std::cout<< " Hello String-Based VideoData"<<std::endl;
    _video_id = video_id;
    _inputVideo =  cv::VideoCapture(video_id);

    _start_frame =0;
    _end_frame =0;
    _fps = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
    _video_time = _frame_count/_fps;
    _length = _frame_count;
    _end_frame = _frame_count;

    std::cout<<" Video time in minutes" << _video_time/60<<std::endl;

    // open the default camera
    if(!_inputVideo.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + video_id);
    }

    int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT);

    _temporary_path = "/home/ragaad/vdms-video/tests/db/videos";


}

VideoData::VideoData(void* buffer, long size, const std::string &path)
{

    std::fstream outfile;
    // _format = VCL::Format::XVID;
    //create a defualt path to store the buffer data
    _temporary_path = path;
    _temporary_path = "/home/ragaad/vdms-video/tests/db/videos";
     //ccreate a unique name of the buffer data
    _temporary_video = VCL::create_unique(path, VCL::Format::AVI);

    //open the outfile inorder to store the buffer data
    outfile.open(_temporary_video, std::ios::out);
    if (outfile.is_open())
    {
    outfile.write(reinterpret_cast<char*>(buffer), size);
    outfile.close();
    _temp_exist = true; // to check the path
    }
    else std::cout<< " The Video Blob was not opened!"<<std::endl;

 //after we recieved the video file, we create a VideoCapture out of it in-order toget the vide details.
    _inputVideo =  cv::VideoCapture(_temporary_video);
    _fps         = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    _video_time = _frame_count/_fps;
    _length = _frame_count;
    _end_frame = _frame_count;
     _start_frame =0;

    // open the default camera
    if(!_inputVideo.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the Video File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + _temporary_video);
    }
    int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT); // to get the current extention of the video file
}


VideoData::VideoData(const cv::VideoCapture &cv_video )
{
    _inputVideo = cv_video;

    if(!_inputVideo.isOpened())  // check if we succeeded
        std::cout<<" Error in Opening the File " <<std::endl;
    else {
        _frame_count = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
        _fps = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
        int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));
        char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
        set_format_from_extension(EXT);
        _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
        _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);
        _end_frame = _frame_count;
        _start_frame =0;

        }

    _temporary_path ="/home/ragaad/vdms-video/tests/db/videos";
    }

void VideoData::read(const std::string &video_id){

   // video_id = create_fullpath(video_id, _format);

    _operations.push_back(std::make_shared<Read> (_video_id, _format));
}

void VideoData::write( const std::string &video_id, Format video_format,
            bool store_metadata)
{
    _operations.push_back(std::make_shared<Write> (create_fullpath(video_id, video_format),
        video_format, _format, store_metadata));
}



void VideoData::operator=(const VideoData &img)
{

}

VideoData::~VideoData()
{
    /** FIX ME **/
    // Delete the temp file we created
    _inputVideo.release();
    // if ( _infile != nullptr && _infile->is_open() )
    //     _infile->close();
    // if ( _outfile != nullptr && _outfile->is_open() )
    //     _outfile->close();

    // if ( _temp_exist ) {
    //     std::string temp_cmd = "rm " + _temporary_path + "*.*";
    //     system(temp_cmd.c_str());
    // }
  // Closes all the windows

}


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string VideoData::get_video_id() const
{
    return _video_id;
}

Format VideoData::get_video_format() const
{
    return _format;
}

long VideoData::get_frame_count() const
{
    return _frame_count;
}

int VideoData::get_type() const
{
    return _cv_type;
    return 0;
}

cv::Size VideoData::get_dimensions()
{
     _size = cv::Size((int) _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                 (int) _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
    return _size;
}

cv::Size VideoData::get_size()
{
   _size = cv::Size(_frame_width,    // Acquire input size
                _frame_height);
    return _size;


}

void VideoData::get_buffer(void* buffer, int buffer_size)
{

    perform_operations();

    //copy_to_buffer(static_cast<unsigned short*>(buffer));
}


cv::VideoCapture VideoData::get_cv_video_capture()
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

     return area;
}

char* VideoData::get_encoded(VCL::Format format,
    const std::vector<int>& params)
{



    perform_operations();
    std::string extension = "." + format_to_string(format);
    std::ifstream ifile;
    ifile.open(_temporary_video, std::ifstream::in);

    char* inBuf;
    ifile.seekg(0, std::ios::end);
    _encoded_size = (long)ifile.tellg();
    ifile.seekg(0, std::ios::beg);
    inBuf = new char[_encoded_size];
    ifile.read(inBuf, _encoded_size);
    // std::string json_query = std::string(inBuf);
    ifile.close();


    // cv::imencode(extension, _cv_img, buffer, params);

    // _encoded_video = new unsigned char[fsize];
    // std::memcpy(_encoded_video, inBuf, fsize);
    // std::vector<unsigned char> buffer;
    // buffer.push_back(*_encoded_video);

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
void VideoData::create_unique(const std::string &path,
    Format format)
{
    _video_id = VCL::create_unique(path, format);
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
        _format = VCL::Format::AVI;
    else if ( extent == "mp4" || extent == "fmp4" )
        _format = VCL::Format::MP4;
    else if ( extent == "mpeg" )
        _format = VCL::Format::MPEG;
    else if ( extent == "xvid" )
        _format = VCL::Format::XVID;
    else
        throw VCLException(UnsupportedFormat, extension + " is not a " +
            "supported format");
}

void VideoData::set_format(int form)
{
    switch (form) {
        case 4:
            _format = VCL::Format::MP4;
            break;
        case 5:
            _format = VCL::Format::AVI;
            break;
        case 6:
            _format = VCL::Format::MPEG;
            break;
        default:
            throw VCLException(UnsupportedFormat, form + " is not a \
            supported video format");
    }
}

void VideoData::set_type(int type)
{
     _cv_type = type;

    // _channels = (type / 8) + 1;
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

void VideoData::set_data_from_raw(void* buffer, int size)
{


}

void VideoData::set_data_from_encoded(const std::vector<unsigned char> &buffer)
{

}

void VideoData::set_minimum(int dimension)
{

}




void VideoData::set_compression(CompressionType comp)
{
    _compress = comp;
}

    /*  *********************** */
    /*   VideoDATA INTERACTION  */
    /*  *********************** */

void VideoData::perform_operations()
{

     for (int x = 0; x < _operations.size(); ++x) {
        std::shared_ptr<Operation> op = _operations[x];
        if ( op == NULL )
            throw VCLException(ObjectEmpty, "Nothing to be done");
        (*op)(this);
    }

    _operations.clear();


}

void VideoData::resize(int rows, int columns)
{

    _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format));
}

void VideoData::interval(std::string unit, int start, int stop, int step)
{
     _video_unit=unit;
    _start_frame = start;
    _end_frame = stop;
    _step = step;
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

}
           /*  *********************** */
            /*         PRIVATE          */
            /*  *********************** */

    /*  *********************** */
    /*      COPY FUNCTIONS      */
    /*  *********************** */


void VideoData::copy_cv(const cv::VideoCapture &video_id)
{

    _inputVideo =  cv::VideoCapture(video_id);


    _fps = _inputVideo.get(CV_CAP_PROP_FPS);
    _frame_count= _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
    _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
    _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);



}

template <class T>
void VideoData::copy_to_buffer(T* buffer)
{

}

    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */

std::string VideoData::create_fullpath(const std::string &filename,
    Format format)
{
    if ( filename == "" )
        throw VCLException(ObjectNotFound, "Location to write object is undefined");

    std::string extension = VCL::get_extension(filename);
    std::string ext = VCL::format_to_string(format);

    if ( ext.compare(extension) == 0 || ext == "" )
        return filename;
    else
        return filename + "." + ext;


}

