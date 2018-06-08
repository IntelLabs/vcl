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



Video:: Video (void* buffer, int size )
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


