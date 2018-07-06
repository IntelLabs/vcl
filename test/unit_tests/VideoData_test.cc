/**
 * @file   ImageData_test.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "VideoData.h"
#include "gtest/gtest.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <string>
#include <iostream>
#include <fstream>
using namespace std;


long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

class VideoDataTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        _video = "test/videos/pedestrian1.avi";
        _inputVideo = cv::VideoCapture("test/videos/pedestrian1.avi");
    }


    std::string _video;
    cv::VideoCapture _inputVideo;

};



TEST_F(VideoDataTest, DefaultConstructor)
{
    std::cout<<" Hello From Default Constructor" << std::endl;
    VCL::VideoData video_data;
}




TEST_F(VideoDataTest, StringConstructor)
{
    std::cout<< " String Constructor" << std::endl;
    VCL::VideoData video_data(_video);
}


TEST_F(VideoDataTest, ObjectConstructor)
{
    std::cout<< " CVCapture Object Constructor" << std::endl;

    VCL::VideoData video_data(_inputVideo); //
    std::cout<< " The Video Size is " << video_data.get_size();


}

TEST_F(VideoDataTest, BlobConstructor)
{
    std::cout<< " Blob Object Constructor" << std::endl;
    std::ifstream ifile;
    ifile.open(_video, std::ifstream::in);
    ASSERT_FALSE(ifile.fail());

    int fsize;
    char* inBuf;
    ifile.seekg(0, std::ios::end);
    fsize = (long)ifile.tellg();
    ifile.seekg(0, std::ios::beg);
    inBuf = new char[fsize];
    ifile.read(inBuf, fsize);
    // std::string json_query = std::string(inBuf);
    ifile.close();

    VCL::VideoData video_data(inBuf, fsize, "test/temp/"); //
    std::cout<< " The Video Size is " << fsize <<"\t"<< sizeof(inBuf)<<std::endl;
    delete[] inBuf;

}

TEST_F(VideoDataTest, Read)
{
    std::cout<< " Read OPeration" << std::endl;

    VCL::VideoData video_data(_video); //

    video_data.read(_video, 10, 200,1);

    video_data.perform_operations();
     std::cout<< " The Video Size is " << video_data.get_size();


 }

TEST_F(VideoDataTest, Write)
{
    std::cout<< " Write Operation" << std::endl;

    VCL::VideoData video_data(_video); //
    video_data.write(_video, VCL::Format::MP4, true, 10, 200, 1);
  //  video_data.perform_operations();


}

TEST_F(VideoDataTest, Resize)
{
    std::cout<< "Resize Operation" << std::endl;

   VCL::VideoData video_data(_inputVideo); //
    video_data.resize( 100,100, 10, 200,1);
    video_data.set_temporary_directory("test/temp/");
    video_data.perform_operations();
}

TEST_F(VideoDataTest, Threshold)
{
    std::cout<< "threshold Operation" << std::endl;

   VCL::VideoData video_data(_inputVideo); //
    video_data.threshold( 200, 10, 100,1);
    video_data.set_temporary_directory("test/temp/");
    video_data.perform_operations();


}

// TEST_F(VideoDataTest, CreateUnique)
// {
//      VCL::VideoData video_data(_inputVideo); //

//     video_data.create_unique("test/videos/", VCL::Format::MP4);
//     std::cout<<video_data.get_video_id() <<std::endl;

//     video_data.write(video_data.get_video_id(), VCL::Format::MP4);
//     video_data.perform_operations();
// }

TEST_F(VideoDataTest, Crop)
{
    try {
    VCL::VideoData video_data(_inputVideo);
    video_data.set_temporary_directory("test/temp/");

    video_data.crop(VCL::Rectangle(0, 0, 50, 50), 100, 150,1);
    video_data.perform_operations();
    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }
}