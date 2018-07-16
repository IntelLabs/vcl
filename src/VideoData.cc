#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>

using namespace std;

#include "VideoData.h"

using namespace VCL;

VideoData::Read::Read(const std::string& filename, Format format, int start, int stop, int step)
    : Operation(format),
      _fullpath(filename),
      _start(start),
      _stop(stop),
      _step(step)
{
}

void VideoData::Read::operator()(VideoData *video)
{
     std::cout<<" Read Operation Inside"<<std::endl;
     video->_inputVideo.set(CV_CAP_PROP_POS_FRAMES, _start - 1);
     std::cout<<"After set operation"<<std::endl;
    int count = _start;

    while ( count < _stop ) {
        cv::Mat frame;
        std::cout<<" Inside Reading Loop"<<std::endl;
        if ( video->_inputVideo.read(frame) ) {
            if ( !frame.empty() ) {
                cv::imshow( "Frame", frame );
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
    Format old_format, bool metadata, int start, int stop, int step)
    : Operation(format),
      _old_format(old_format),
      _metadata(metadata),
      _fullpath(filename),
      _start(start),
      _stop(stop),
      _step(step)
{
}

void VideoData::Write::operator()(VideoData *video)
{
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

void VideoData::Resize::operator()(VideoData *video)
{
     std::cout<<" Hello I am in the Resize";
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
    video->create_unique(video->_temporary_path, VCL::Format::AVI);

    std::string cropped_vid = video->remove_extention(video->_video_id) + "_cropped.avi" ;
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
    video->create_unique(video->_temporary_path, VCL::Format::AVI);
    std::string cropped_vid = video->remove_extention(video->_video_id) + "threshold.avi" ;
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

    std::cout<< "Hello"<<std::endl;
    _width = 0;
    _height =0;
    _length=0;
    _start_frame =0;
    _end_frame =0;
    _fps=0;
    _frame_width=0;
    _frame_height=0;
    _video_id ="";
    _format = VCL::Format::NONE;
    _temporary_path = "temp/";
}


VideoData::VideoData(const VideoData &video)
{
     copy_cv(_inputVideo);

     _format = video._format;
    // _compress = VCL::CompressionType::LZ4;
     _video_id = "";


}

VideoData::VideoData( const std::string &video_id )
{

    std::cout<< " Hello String-Based VideoData"<<std::endl;
    _video_id = video_id;
    _inputVideo =  cv::VideoCapture(video_id);

    _fps = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    _video_time = _frame_count/_fps;
    _length = _frame_count;

    std::cout<<" Video time in minutes" << _video_time/60<<std::endl;

    // open the default camera
    if(!_inputVideo.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + video_id);
    }

    int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT);
    // std::cout<< (int)_format <<"\t"<< _length<<std::endl;
    _temporary_path = "temp/";

 // while(1)
 //  {
 //    cv::Mat frame;

 //    // Capture frame-by-frame
 //    _inputVideo >> frame;

 //    // If the frame is empty, break immediately
 //    if (frame.empty())
 //      break;

 //    // Write the frame into the file 'outcpp.avi'
 //    _outputVideo.write(frame);

 //    // Display the resulting frame
 //    //cv::imshow( "Frame", frame );

 //    // Press  ESC on keyboard to  exit
 //    char c = (char)cv::waitKey(1);
 //    if( c == 27 )
 //      break;
 //  }



   // string::size_type pAt = source.find_last_of('.');                  // Find extension point


}

VideoData::VideoData(void* buffer, long size, const std::string &path)
{
    // _encoded_video = new unsigned char[size];
    // std::memcpy(_encoded_video, buffer, size);
    std::fstream outfile;
    // _format = VCL::Format::XVID;
    //create a defualt path to store the buffer data
    _temporary_path = path;
     //ccreate a unique name of the buffer data
    std::string temp = VCL::create_unique(path, VCL::Format::AVI);

    //open the outfile inorder to store the buffer data
    outfile.open(temp, std::ios::out);
    outfile.write(reinterpret_cast<char*>(buffer), size);
    outfile.close();
    _temp_exist = true; // to check the path

 //after we recieved the video file, we create a VideoCapture out of it in-order toget the vide details.
    _inputVideo =  cv::VideoCapture(temp);

    _fps         = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    _video_time = _frame_count/_fps;
    _length = _frame_count;

   // std::cout<<" Video time in minutes " << _video_time/60.0 <<std::endl;

    // open the default camera
    if(!_inputVideo.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the Video File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + temp);
    }

    int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT); // to get the current extention of the video file
}


VideoData::VideoData(const cv::VideoCapture &cv_video )
{
    std::cout<<" Here is the CV VideoCapture CONSTRUCTORS"<<std::endl;

    _inputVideo = cv_video;

    if(!_inputVideo.isOpened())  // check if we succeeded
        std::cout<<" Error in Opening the File " <<std::endl;
    else {
        _length = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
        _fps = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
        int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));
        char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
        set_format_from_extension(EXT);
        _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
        _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

        }

    _temporary_path = "temp/";
    std::vector<cv::Mat> one_video_vector;

    // cv::Mat video_img;
    // cv::VideoCapture Temp_video_capture=_inputVideo;
    // Temp_video_capture.read(video_img);
    // while (!video_img.empty())
    // {
    //     one_video_vector.push_back(video_img);
    //     Temp_video_capture.read(video_img);
    // }

    // Temp_video_capture.release();

    // std::cout<< " The number of the frames is " <<one_video_vector.size();



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

void VideoData::read(const std::string &video_id, int start , int stop, int step){


   // video_id = create_fullpath(video_id, _format);

    _operations.push_back(std::make_shared<Read> (_video_id, _format, start, stop, step));


}

void VideoData::write( const std::string &video_id, Format video_format,
            bool store_metadata, int start, int stop, int step)
{
    _operations.push_back(std::make_shared<Write> (create_fullpath(video_id, video_format),
        video_format, _format, store_metadata, start, stop, step));
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

std::vector<unsigned char> VideoData::get_encoded(Format format,
    const std::vector<int>& params)
{

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

void VideoData::resize(int rows, int columns, int start , int stop, int step)
{

    _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format, start, stop,step));
}

void VideoData::interval(int start, int stop, int step)
{
 _operations.push_back(std::make_shared<Interval> (start, stop, step, _format));
}

void VideoData::crop(const Rectangle &rect, int start, int stop, int step)
{
 _operations.push_back(std::make_shared<Crop> (rect, _format, start, stop, step));
}

void VideoData::threshold(int value, int start, int stop , int step )
{
_operations.push_back(std::make_shared<Threshold> (value, _format, start, stop, step));
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