#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <iostream>
#include <fstream>
using namespace std;

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

    std::cout<< "Hello"<<std::endl;
    // _channels = 0;
    // _height = 0;
    // _width = 0;
    // _cv_type = CV_8UC3;

    // _format = VCL::NONE;
    // _compress = VCL::CompressionType::LZ4;

    // _tdb = NULL;
    // _Video_id = "";
}

VideoData::VideoData(const VideoData &video)
{
    // copy_cv(cv_img);

    // _format = VCL::NONE;
    // _compress = VCL::CompressionType::LZ4;
    // _Video_id = "";

    // _tdb = NULL;
}

VideoData::VideoData( const std::string &video_id)
{


   /*

vector<Mat>load_single_video(string path)
{
    vector<Mat> one_video_vector;
    Mat video_img;
    VideoCapture Temp_video_capture(path);
    Temp_video_capture.read(video_img);
    while (!video_img.empty())
    {
        one_video_vector.push_back(video_img);
        Temp_video_capture.read(video_img);
    }

    Temp_video_capture.release();

    return one_video_vector;
}


   */




     std::cout<< " Hello VidoeData"<<std::endl;
    _inputVideo =  cv::VideoCapture(video_id);

    _fps = _inputVideo.get(CV_CAP_PROP_FPS);
    _frame_count= _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
    _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
    _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);
     _video_time= _frame_count/_fps;

    std::cout<<" Video time in minutes" << _video_time/60<<std::endl;


    // open the default camera
    if(!_inputVideo.isOpened())  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;


    _video_id = video_id;


   _format1 = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // Transform from int to char via Bitwise operators
   //char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};


    _length = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
     std::cout<< _format1 <<"\t"<< _length<<std::endl;

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
void VideoData::read(const std::string &video_id){

infile = new std::ifstream(video_id,std::ifstream::binary);
infile->seekg (0,infile->end);
long size = infile->tellg();
infile->seekg (0);

  // allocate memory for file content
char* buffer = new char[size];

  // read content of infile
  infile->read (buffer,size);

   delete[] buffer;

   infile->close();


}

void VideoData::write( const std::string &video_id,
            bool store_metadata)
{
infile = new std::ifstream (video_id,std::ifstream::binary);
outfile = new std::ofstream("v.tex",std::ofstream::binary);

  // get size of file
  infile->seekg (0,infile->end);
  long size = infile->tellg();
  infile->seekg (0);

  // allocate memory for file content
  char* buffer = new char[size];

  // read content of infile
  infile->read (buffer,size);

  // write to outfile
  outfile->write (buffer,size);

  // release dynamically-allocated memory
  delete[] buffer;

  outfile->close();
  infile->close();



}

VideoData::VideoData(void* buffer, int size)
{

    std::cout<<" This is the blob Constructor"<<std::endl;
    std::cout<< " The BloB size is "<< size << std::endl;

}


VideoData::VideoData(const cv::VideoCapture &cv_video )
{

 std::cout<<" Here is the CV VideoCapture CONSTRUCTORS"<<std::endl;

 _inputVideo = cv_video;

  if(!_inputVideo.isOpened())  // check if we succeeded
      std::cout<<" Error in Opening the File " <<std::endl;
  else {


     _length = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
     _format1 = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));

      _frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
      _frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

      std::cout<< " Frame Width is" << _frame_width << std::endl;
      std::cout<< " Frame Height is" << _frame_height<<std::endl;
    }

    /*

  std::vector<cv::Mat> one_video_vector;

    cv::Mat video_img;
    cv::VideoCapture Temp_video_capture(video_id);
    Temp_video_capture.read(video_img);
    while (!video_img.empty())
    {
        one_video_vector.push_back(video_img);
        Temp_video_capture.read(video_img);
    }

    Temp_video_capture.release();

    std::cout<< " the number of the frames is " <<one_video_vector.size();


    */
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

}

VideoData::~VideoData()
{

 // _inputVideo.release();
 //  _outputVideo.release();

 //  // Closes all the windows
 //  destroyAllWindows();
 //  delete _video;
}


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

std::string VideoData::get_Video_id() const
{
    return _video_id;
}

VideoFormat VideoData::get_Video_format() const
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

     return area;
}

std::vector<unsigned char> VideoData::get_encoded(VideoFormat format,
    const std::vector<int>& params)
{

}


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
void VideoData::create_unique(const std::string &path,
    VideoFormat format)
{

}

void VideoData::set_Video_id(const std::string &video_id)
{
  _video_id =video_id;
}

void VideoData::set_format(const std::string &extension)
{
    if ( extension == "avi" )
        _format = VCL::AVI;
    else if ( extension == "mp4" )
        _format = VCL::MP4;
    else if ( extension == "mpeg" )
        _format = VCL::MPEG;
    else
        throw VCLException(UnsupportedFormat, extension + " is not a \
            supported format");
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

}

void VideoData::resize(int rows, int columns)
{

}

void VideoData::crop(const Rectangle &rect)
{

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

void VideoData::copy_cv(const cv::Mat &cv_img)
{

}

template <class T>
void VideoData::copy_to_buffer(T* buffer)
{

}

    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */
std::string VideoData::get_extension(const std::string &Video_id)
{
    size_t file_ext = Video_id.find_last_of(".");
    size_t dir_ext = Video_id.find_last_of("/");

    if ( file_ext != std::string::npos ) {
        if ( file_ext > dir_ext + 2 )
            return Video_id.substr(file_ext + 1);
        else
            throw VCLException(ObjectEmpty, Video_id + " does not have a valid extension");
    }
    else
        return "";

}

std::string VideoData::format_to_string(VideoFormat Video_format)
{

    return " Format";
}

std::string VideoData::create_fullpath(const std::string &filename,
    VideoFormat format)
{

    return "id";
}

bool VideoData::exists(const std::string &name)
{

    return true;
}
