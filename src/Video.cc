#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <iostream>

#include "Video.h"
#include "VideoData.h"


using namespace VCL;

using namespace cv; // OpenCV namespace

void Video::store(const std::string &video_id, Format video_format,
            bool store_metadata)
{
  _video->write(video_id, video_format, store_metadata);
  _video->perform_operations();
}

std::string Video::create_unique(const std::string &path,
                Format format)
{
  _video->create_unique(path, format);
  return _video->get_video_id();
}

Video::Video()
{
  std::cout<<"Empty Constructor"<<std::endl;

  _video = new VideoData();
}

Video::Video(const std::string &fileName)
{

  _video = new VideoData(fileName);


}



Video::Video(const cv::VideoCapture video)
{

  _video = new VideoData(video);


}


Video::Video(void* buffer, int size , const std::string &path)
{
  _video = new VideoData(buffer, size, path);
}

Video::Video( const Video &video){

  _video = new VideoData(*video._video);

}

 void Video::operator=(const Video &vid){

     VideoData *temp = _video;
    _video = new VideoData(*vid._video);
    delete temp;

 }



Video::~Video()
{

delete _video;

}
 /***********Getter Functions ****************/
std::string Video::get_video_id() const
{
  return _video->get_video_id();

}
long Video::get_frame_count() const
{
  return _video->get_frame_count();
}

cv::Size Video::get_dimensions() const
{
    return _video->get_dimensions();
}


void Video::delete_video()
{

    delete _video;

}

Format Video::get_video_format() const
{
    return _video->get_video_format();
}

int Video::get_video_type() const
{
    return _video->get_type();
}


void Video::get_raw_data(void* buffer, long buffer_size ) const
{
    _video->get_buffer(buffer, buffer_size);
}
/**********************************/

void Video::set_compression(CompressionType comp)
{
    _video->set_compression(comp);
}


void Video::set_dimensions(cv::Size dims)
{
    _video->set_dimensions(dims);
}

void Video::set_video_type(int cv_type)
{
    _video->set_type(cv_type);
}

void Video::set_minimum_dimension(int dimension)
{
    _video->set_minimum(dimension);
}

/********************OPERATIONS ***********************/
void Video::resize(int new_height, int new_width)
{
    _video->resize(new_height, new_width);
    // _video->perform_operations();
}

void Video::interval(std::string unit, int start, int stop, int step)
{
    _video->interval(unit, start, stop, step);
    // _video->perform_operations();
}

void Video::crop(const Rectangle &rect)
{
    _video->crop(rect);
    // _video->perform_operations();
}

void Video::threshold(int value)
{
   _video->threshold(value);
   // _video->perform_operations();
}


