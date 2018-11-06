#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <iostream>

#include "Video.h"
#include "VideoData.h"

using namespace VCL;
using namespace cv; // OpenCV namespace

Video::Video()
{
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
void Video::store(const std::string &video_id, VCL::Video::Format video_format)
{
  _video->write(video_id, video_format);
  _video->perform_operations();
}
std::string Video::create_unique(const std::string &path,
               VCL::Video::Format format)
{
  _video->create_unique(path, format);
  return _video->get_video_id();
}
char* Video::get_encoded_video(VCL::Video::Format format,
                const std::vector<int>& params) const
{
  return _video->get_encoded(format, params);
}
long Video::get_encoded_size()
{
  return _video->get_size_encoded();
}

Video::Video(void* buffer, int size )
{
  _video = new VideoData(buffer, size);
}

Video::Video( const Video &video)
{
  _video = new VideoData(*video._video);
}

void Video::operator=(const Video &vid)
{
     delete _video;
    _video = new VideoData(*vid._video);
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
VCL::Video::Video_Size Video::get_size() const
{
 return _video->get_size();
}

void Video::delete_video()
{
    delete _video;//delete video files
}
VCL::Video::Format Video::get_video_format() const
{
    return _video->get_video_format();
}

/********************OPERATIONS ***********************/
void Video::resize(int new_height, int new_width)
{
    _video->resize(new_height, new_width);
}

void Video::interval(Video::UNIT u, int start, int stop, int step)
{
    _video->interval(u, start, stop, step);
}
void Video::crop(const Rectangle &rect)
{
    _video->crop(rect);
}
void Video::threshold(int value)
{
   _video->threshold(value);
}