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


Video::Video(const std::string &fileName)
{

  std::cout<< " File name is " << fileName << std::endl;

}



Video::Video(void* buffer, int size )
{
  _video = new VideoData(buffer, size);
}

 void Video::operator=(const Video &vid){

    //  VideoData *temp = _video;
    // _video = new VideoData(*vid._video);
    // delete temp;

 }



Video::~Video()
{

}

std::string Video::get_video_id() const
{
  return "id";

}

void Video::delete_video()
{
    //_video->delete_object();

  //  delete _video;

   // _video = new VideoData;
}

void Video::resize(int new_height, int new_width)
{
    _video->resize(new_height, new_width, 10, 100);
}

void Video::interval(int from, int to)
{
    _video->interval(from, to);
}

void Video::crop(const Rectangle &rect, int start, int stop)
{
    _video->crop(rect, start, stop);
}

void Video::threshold(int value)
{
   //_video->threshold(value);
}


