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
#include <opencv2/highgui/highgui.hpp>

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
        _video = "videos/Megamind.avi";
        _inputVideo = cv::VideoCapture("videos/Megamind.avi");

    }
    void compare_mat_mat(cv::Mat &cv_img, cv::Mat &img)
    {
        int rows = img.rows;
        int columns = img.cols;
        int channels = img.channels();

        if ( img.isContinuous() ) {
            columns *= rows;
            rows = 1;
        }

        for ( int i = 0; i < rows; ++i ) {
            for ( int j = 0; j < columns; ++j ) {
                if (channels == 1) {
                    unsigned char pixel = img.at<unsigned char>(i, j);
                    unsigned char test_pixel = cv_img.at<unsigned char>(i, j);
                    ASSERT_EQ(pixel, test_pixel);
                }
                else {
                    cv::Vec3b colors = img.at<cv::Vec3b>(i, j);
                    cv::Vec3b test_colors = cv_img.at<cv::Vec3b>(i, j);
                    for ( int x = 0; x < channels; ++x ) {
                        ASSERT_EQ(colors.val[x], test_colors.val[x]);
                    }
                }
            }
        }
    }
    void compare_cvcapture_cvcapture(cv::VideoCapture v1, cv::VideoCapture v2, int start, int end){
        int count=start;
        while ( count < end ) {
        cv::Mat frame1;
        cv::Mat frame2;
        if ( v1.read(frame1) && v2.read(frame2)) {
            if ( !frame1.empty() && !frame2.empty()) {

                 compare_mat_mat(frame1,frame2);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=1;
    }

    }


    VCL::Rectangle rect_;
    VCL::Rectangle bad_rect_;
    std::string img_;
    std::string tdb_img_;

    cv::Mat cv_img_;

    int dimension_;
    int size_;

    std::string _video;
    cv::VideoCapture _inputVideo;
   };


TEST_F(VideoDataTest, DefaultConstructor)
{
    VCL::VideoData video_data;
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;
    cv::VideoCapture testVideo = cv::VideoCapture();
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, StringConstructor)
{
    std::cout<< " String Constructor" << std::endl;
    VCL::VideoData video_data(_video);
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;

    cv::VideoCapture testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, ObjectConstructor)
{
    std::cout<< " CVCapture Object Constructor" << std::endl;
    VCL::VideoData video_data(_inputVideo); //
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;

    cv::VideoCapture testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, BlobConstructor)
{
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
    ifile.close();
    VCL::VideoData video_data(inBuf, fsize); //
    delete[] inBuf;

    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;
    cv::VideoCapture testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video_data.get_cv_video(), testVideo, 1, test_frame_count);
}

TEST_F(VideoDataTest, Interval)
{
    try {
    std::cout<< " Interval Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval("frames",10, 200, 5);
    video_data.write(_video , VCL::VideoFormat::XVID);
    video_data.perform_operations();
    long input_frame_count = video_data.get_output_video().get(CV_CAP_PROP_FRAME_COUNT);
    long test_frame_count;
    cv::VideoCapture testVideo = cv::VideoCapture("videos/Megamind.avi");
    cv::VideoWriter testResultVideo = cv::VideoWriter("test_interval.avi",
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    testVideo.get(CV_CAP_PROP_FPS),
                    cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 10;
    int _stop = 200;
    int _step = 5;
    while ( count < _stop ) {
        cv::Mat frame;
        if ( testVideo.read(frame) ) {
            if ( !frame.empty() ) {

                 testResultVideo.write(frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }

    test_frame_count = testResultVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video_data.get_cv_video(), testVideo, 1, test_frame_count);

    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }

}

TEST_F(VideoDataTest, Read)
{
   try
   {
    std::cout<< " Read Operation" << _video<< std::endl;
    VCL::VideoData video_data(_video);
    video_data.interval("frames",10, 200, 5);
    video_data.read(_video);
    video_data.perform_operations();
   }
  catch(VCL::Exception &e) {
        print_exception(e);
    }

 }

TEST_F(VideoDataTest, Write)
{
    try {
    std::cout<< " Write Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval("frames", 10, 200, 5);
    video_data.write(_video, VCL::VideoFormat::XVID);
    video_data.perform_operations();
    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }

}

TEST_F(VideoDataTest, Resize)
{
    try
    {
        std::cout<< "Resize Operation" << std::endl;
        VCL::VideoData video_data(_inputVideo);
        video_data.interval("frames",10, 200, 5);
        video_data.set_temporary_directory("temp/");
        video_data.resize( 100,100);
        video_data.perform_operations();
    }
  catch(VCL::Exception &e)
  {
        print_exception(e);
  }
}

TEST_F(VideoDataTest, Threshold)
{
   try
   {
    std::cout<< "threshold Operation" << std::endl;
    VCL::VideoData video_data(_inputVideo);
    video_data.set_temporary_directory("temp/");
    video_data.interval("frames",10, 200, 5);
    video_data.threshold( 100);
    video_data.set_temporary_directory("temp/");
    video_data.perform_operations();
   }
catch(VCL::Exception &e) {
        print_exception(e);
    }

}

TEST_F(VideoDataTest, CreateUnique)
{
    try
     {
        VCL::VideoData video_data(_inputVideo); //
        video_data.create_unique("temp/", VCL::VideoFormat::MP4);
        std::cout<<video_data.get_video_id() <<std::endl;
        video_data.interval("frames",10, 200, 5);
        video_data.write(video_data.get_video_id(), VCL::VideoFormat::MP4);
        video_data.perform_operations();
    }
catch(VCL::Exception &e) {
        print_exception(e);
    }
}

TEST_F(VideoDataTest, Crop)
{
    try
    {
        VCL::VideoData video_data(_inputVideo);
        video_data.set_temporary_directory("temp/");
        video_data.interval("frames", 10, 200, 5);
        video_data.crop(VCL::Rectangle(0, 0, 50, 50));
        video_data.perform_operations();
    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }
}
