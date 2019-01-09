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



#include "helpers.h"

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

    VCL::Rectangle rect_;
    VCL::Rectangle bad_rect_;
    std::string img_;
    std::string tdb_img_;
    cv::Mat cv_img_;
    int dimension_;
    int size_;
    std::string _video ;
    cv::VideoCapture _inputVideo;
    cv::VideoCapture testVideo;
   };


TEST_F(VideoDataTest, DefaultConstructor)
{
    VCL::VideoData video_data;
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;
    cv::VideoCapture testVideo = cv::VideoCapture();
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, StringConstructor)
{
    std::cout<< " String Constructor" << std::endl;
    VCL::VideoData video_data(_video);
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, ObjectConstructor)
{
    std::cout<< " CVCapture Object Constructor" << std::endl;
    VCL::VideoData video_data(_inputVideo); //
    long input_frame_count = video_data.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoDataTest, BlobConstructor)
{
    std::ifstream ifile;
    ifile.open(_video);
     if (!ifile.is_open())
            std::cout<<"Error Opening the video File";
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
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count = testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video_data.get_cv_video(), testVideo, 1, test_frame_count);
}


TEST_F(VideoDataTest, Read)
{
   try
   {
    std::cout<< " Read Operation" << _video<< std::endl;
    VCL::VideoData video_data(_video);
    video_data.read(_video);
    video_data.perform_operations();
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count = video_data.get_frame_count();
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video_data.get_cv_video(), testVideo, 1, test_frame_count);
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
    video_data.write(_video, VCL::Video::Format::XVID);
    video_data.perform_operations();
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
     cv::VideoWriter testResultVideo = cv::VideoWriter("videos_test/test_write.avi",
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    testVideo.get(CV_CAP_PROP_FPS),
                    cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 0;
    int _stop = video_data.get_frame_count();
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
    cv::VideoCapture test_written_video("videos_test/test_write_write.avi");
    std::string cv_name ;
    cv_name = video_data.get_fullpath();
    cv::VideoCapture vcl_written_video(cv_name);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
    catch(VCL::Exception &e) {
        print_exception(e);
    }

}
TEST_F(VideoDataTest, Interval)
{
    try {
    std::cout<< " Interval Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video_data.write(_video, VCL::Video::Format::XVID);
    video_data.perform_operations();
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
     cv::VideoWriter testResultVideo = cv::VideoWriter("videos_test/test_interval.avi",
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
    cv::VideoCapture test_written_video("videos_test/test_write_interval.avi");
    std::string cv_name ;
    cv_name = video_data.get_fullpath();
    cv::VideoCapture vcl_written_video(cv_name);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
    catch(VCL::Exception &e) {
        print_exception(e);
    }


}



TEST_F(VideoDataTest, Resize)
{
    try {
    std::cout<< " Resize Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video_data.resize( 10,10);
    video_data.write(_video, VCL::Video::Format::XVID);

    video_data.perform_operations();

    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
     cv::VideoWriter testResultVideo = cv::VideoWriter("videos_test/test_resize.avi",
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
                cv::Mat cv_resized;
                cv::resize(frame, cv_resized, cv::Size(10, 10));
                testResultVideo.write(frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }
    cv::VideoCapture test_written_video("videos_test/test_write_resize.avi");
    std::string cv_name ;
   // std::cout<< _video.get_fullpath() <<std:endl;
    cv_name = video_data.get_fullpath();
    cv::VideoCapture vcl_written_video(cv_name);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
  catch(VCL::Exception &e)
  {
        print_exception(e);
  }
}

TEST_F(VideoDataTest, Threshold)
{
   try {
    std::cout<< " Threshold Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video_data.threshold( 150);
    video_data.write(_video, VCL::Video::Format::XVID);
    video_data.perform_operations();
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
     cv::VideoWriter testResultVideo = cv::VideoWriter("videos_test/test_threshold.avi",
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

                 cv::threshold(frame, frame, 150, 150,
                 cv::THRESH_TOZERO);

                 testResultVideo.write(frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }
    cv::VideoCapture test_written_video("videos_test/test_write_threshold.avi");
    std::string cv_name ;
   // std::cout<< _video.get_fullpath() <<std:endl;
    cv_name = video_data.get_fullpath();
    cv::VideoCapture vcl_written_video(cv_name);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
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
        video_data.create_unique("videos_test/", VCL::Video::Format::MP4);
        std::cout<<video_data.get_video_id() <<std::endl;
        video_data.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
        video_data.write(video_data.get_video_id(), VCL::Video::Format::MP4);
        video_data.perform_operations();
    }
catch(VCL::Exception &e) {
        print_exception(e);
    }
}

TEST_F(VideoDataTest, Crop)
{

   try {
    std::cout<< " Crop Operation" << std::endl;
    VCL::VideoData video_data(_video); //
    video_data.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video_data.crop(VCL::Rectangle(0, 0, 50, 50));
    video_data.write(_video, VCL::Video::Format::XVID);
    video_data.perform_operations();
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
     cv::VideoWriter testResultVideo = cv::VideoWriter("videos_test/test_crop.avi",
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
                 if ( frame.rows < 50 + 0 || frame.cols < 50 + 0 )
                    throw VCLException(SizeMismatch, "Requested area is not within the Video");
                cv::Mat roi_frame(frame, VCL::Rectangle(0, 0, 50, 50));
                testResultVideo.write(roi_frame);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
    }
    cv::VideoCapture test_written_video("videos_test/test_write_crop.avi");
    std::string cv_name ;

    cv_name = video_data.get_fullpath();
    cv::VideoCapture vcl_written_video(cv_name);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }

catch(VCL::Exception &e) {
        print_exception(e);
    }


}
