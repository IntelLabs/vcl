#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <iostream>
#include <fstream>
#include <cstring>

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

        video->copy_cv(video->_inputVideo);
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
    cv::VideoWriter _outputVideo(_fullpath,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    video->_fps,
                    cv::Size(video->_frame_width, video->_frame_height));
    while(1)
    {
        cv::Mat frame;

        // Capture frame-by-frame
        video->_inputVideo >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        // Write the frame into the file 'outcpp.avi'
        _outputVideo.write(frame);

        // Display the resulting frame
        //cv::imshow( "Frame", frame );

        // Press  ESC on keyboard to  exit
        // char c = (char)cv::waitKey(1);
        // if( c == 27 )
        //     break;
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

void VideoData::Crop::operator()(VideoData *video)
{

        // if ( !img->_cv_img.empty() ) {
        //     if ( img->_cv_img.rows < _rect.height + _rect.y || img->_cv_img.cols < _rect.width + _rect.x )
        //         throw VCLException(SizeMismatch, "Requested area is not within the Video");
        //     cv::Mat roi_img(img->_cv_img, _rect);
        //     img->copy_cv(roi_img);
        // }
        // else
            // throw VCLException(ObjectEmpty, "Video object is empty");

}

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */

void VideoData::Threshold::operator()(VideoData *video)
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

    std::cout<< " Hello VideoData"<<std::endl;
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

VideoData::VideoData(void* buffer, long size)
{
    // _encoded_video = new unsigned char[size];
    // std::memcpy(_encoded_video, buffer, size);
    std::ofstream outfile;
    // _format = VCL::Format::XVID;

    std::string temp = VCL::create_unique(_temporary_path, VCL::Format::XVID);

    outfile.open(temp, std::ofstream::binary);
    outfile.write(reinterpret_cast<char*>(buffer), size);
    outfile.close();

    _inputVideo =  cv::VideoCapture(temp);

    _fps = static_cast<float>(_inputVideo.get(CV_CAP_PROP_FPS));
    _frame_count = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_COUNT));
    _frame_width = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_WIDTH));
    _frame_height = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    _video_time = _frame_count/_fps;
    _length = _frame_count;

    std::cout<<" Video time in minutes " << _video_time/60.0 <<std::endl;

    // open the default camera
    if(!_inputVideo.isOpened()) {  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
      throw VCLException(OpenFailed, "Could not open " + temp);
    }

    int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
    set_format_from_extension(EXT);
}


VideoData::VideoData(const cv::VideoCapture &cv_video )
{
    std::cout<<" Here is the CV VideoCapture CONSTRUCTORS"<<std::endl;

    _inputVideo = cv_video;

    if(!_inputVideo.isOpened())  // check if we succeeded
        std::cout<<" Error in Opening the File " <<std::endl;
    else {
        _length = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
        int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));
        char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
        set_format_from_extension(EXT);

        _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
        _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

            }


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

void VideoData::read(const std::string &video_id){


    //video_id = create_fullpath(video_id, _format);

    _operations.push_back(std::make_shared<Read> (_video_id, _format));



// _infile = new std::ifstream(video_id,std::ifstream::binary);
// _infile->seekg (0,_infile->end);
// long size = _infile->tellg();
// _infile->seekg (0);

//   // allocate memory for file content
// char* buffer = new char[size];

//   // read content of infile
//   _infile->read (buffer,size);

//    delete[] buffer;

//    _infile->close();


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
 // _inputVideo.release();
 //  _outputVideo.release();

 //  // Closes all the windows
 //  destroyAllWindows();
 //  delete _video;
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

    // perform_operations();
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
    std::cout<< " Hello VidoeData"<<std::endl;
    _operations.push_back(std::make_shared<Resize> (Rectangle(0, 0, columns, rows), _format));
}

void VideoData::interval(int from, int to)
{
 _operations.push_back(std::make_shared<Interval> (from, to, _format));
}

void VideoData::crop(const Rectangle &rect, int start, int stop)
{
 _operations.push_back(std::make_shared<Crop> (rect, _format, start, stop));
}

void VideoData::threshold(int value)
{

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