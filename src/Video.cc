#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <iostream>

#include "Video.h"


using namespace VCL;

 using namespace cv; // OpenCV namespace
void VCL::Video::store(const std::string &video_id, VideoFormat video_format,
            bool store_metadata)
{


}




Video::Video(const std::string &fileName)
{

  std::cout<< " File name is " << fileName << std::endl;

}



Video::Video (void* buffer, int size )
{
// _video = new VideoData( buffer, size);
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
 //   _video->resize(new_height, new_width);
}

void Video::interval(int from, int to)
{
  //  _video->interval(from, to);
}

void Video::crop(const Rectangle &rect)
{
  //  _video->crop(rect);
}

void Video::threshold(int value)
{
   //_video->threshold(value);
}


