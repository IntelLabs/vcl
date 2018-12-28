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
#include "Video.h"



class VideoTest : public ::testing::Test {
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
    std::string _video ;
    cv::VideoCapture _inputVideo;
    cv::VideoCapture testVideo;
   };


TEST_F(VideoTest, DefaultConstructor)
{
    VCL::Video video;
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    cv::VideoCapture testVideo = cv::VideoCapture();
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoTest, StringConstructor)
{
    std::cout<< " String Constructor" << std::endl;
    VCL::Video video(_video);
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoTest, ObjectConstructor)
{
    std::cout<< " CVCapture Object Constructor" << std::endl;
    VCL::Video video(_inputVideo); //
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);

    std::cout<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    
    std::cout<<video.get_cv_video().get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;

    std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoTest, BlobConstructor)
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
    VCL::Video video(inBuf, fsize); //
    delete[] inBuf;
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    test_frame_count = testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video.get_cv_video(), testVideo, 1, test_frame_count);
}


TEST_F(VideoTest, Read)
{
   try
   {
    std::cout<< " Read Operation" << _video<< std::endl;
    VCL::Video video(_video);
    video.read(_video);
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
     input_frame_count = video.get_frame_count();
     ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video.get_cv_video(), testVideo, 1, test_frame_count);
   }
  catch(VCL::Exception &e) {
        print_exception(e);
    }
 }



 TEST_F(VideoTest, CreateUnique)
{
    try
     {
        VCL::Video video(_inputVideo); //
        video.create_unique("videos_test/", VCL::Video::Format::MP4);
        std::cout<<video.get_video_id() <<std::endl;
        video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
        video.store(video.get_video_id(), VCL::Video::Format::MP4);

    }
catch(VCL::Exception &e) {
        print_exception(e);
    }
}
 
 TEST_F(VideoTest, OpenInternalVideo)
{
   try
   {
   VCL::Video video(_inputVideo); //
        video.create_unique("videos_test/", VCL::Video::Format::MP4);
        std::cout<<video.get_video_id() <<std::endl;
        video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
        video.store(video.get_video_id(), VCL::Video::Format::MP4);

    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
     input_frame_count = video.get_frame_count();
     ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video.get_cv_video(), testVideo, 1, test_frame_count);
   }
  catch(VCL::Exception &e) {
        print_exception(e);
    }
 }

TEST_F(VideoTest, Store)
{
    try {
    std::cout<< " Store Operation" << std::endl;
    VCL::Video video(_video); //
    video.store(_video, VCL::Video::Format::XVID);
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    std::string  test_vi_id ="videos_test/test_store_write1.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    testVideo.get(CV_CAP_PROP_FPS),
                    cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 0;
    int _stop = video.get_frame_count();
    int _step = 1;
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
    cv::VideoCapture test_written_video(test_vi_id);
    std::string cv_name ;
   
    cv::VideoCapture vcl_written_video(video.get_cv_video());
    input_frame_count = video.get_frame_count();
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
     std::cout<< "video frame count\t"<<video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_video\t"<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_written_vdeio\t"<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"vcl_vedio\t"<<(video.get_cv_video()).get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
    catch(VCL::Exception &e) {
        print_exception(e);
    }

}
TEST_F(VideoTest, Interval)
{
    try {
    std::cout<< " Interval Operation" << std::endl;
    VCL::Video video(_video); //
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.store(_video, VCL::Video::Format::XVID);

    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    std::string test_vi_id="videos_test/test_Video_interval.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
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
    cv::VideoCapture test_written_video(test_vi_id);
    std::string cv_name ;
    cv_name = video.get_video_id();
    cv::VideoCapture vcl_written_video(video.get_cv_video());
     input_frame_count = video.get_frame_count();
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout<< video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<video.get_cv_video().get(CV_CAP_PROP_FRAME_COUNT)<<std::endl; 
    std::cout<< "video frame count\t"<<video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_video\t"<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_written_vdeio\t"<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"vcl_vedio\t"<<(video.get_cv_video()).get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;

    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
    catch(VCL::Exception &e) {
        print_exception(e);
    }


}

TEST_F(VideoTest, Resize)
{
    try {
    std::cout<< " Resize Operation" << std::endl;
    VCL::Video video(_video); //
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.resize( 10,10);
    video.store(_video, VCL::Video::Format::XVID);
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    std::string test_vi_id="videos_test/test_Video_resize.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
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
    cv::VideoCapture test_written_video(test_vi_id);

    count =10;
    int i=0;


        while ( count < _stop ) {
        cv::Mat frame;
        if ( test_written_video.read(frame) ) {
          std::cout<<count <<"\t"<<i<<std::endl;
            
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step; i++;

    }
    std::string cv_name ;
   // std::cout<< _video.get_temporary_video() <<std:endl;
     cv_name = video.get_video_id();
    input_frame_count = video.get_frame_count();
     cv::VideoCapture vcl_written_video(video.get_cv_video());
    input_frame_count = video.get_frame_count();
     input_frame_count = vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT);
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
      std::cout<< "video frame count\t"<<video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_video\t"<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_written_vdeio\t"<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"vcl_vedio\t"<<(video.get_cv_video()).get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }
  catch(VCL::Exception &e)
  {
        print_exception(e);
  }
}

TEST_F(VideoTest, Threshold)
{
   try {
    std::cout<< " Threshold Operation" << std::endl;
    VCL::Video video(_video); //
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.threshold( 150);
    video.store(_video, VCL::Video::Format::XVID);

    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    std::string test_vi_id="videos_test/test_Video_threshold.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
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
    cv::VideoCapture test_written_video(test_vi_id);
    std::string cv_name ;
   // std::cout<< _video.get_temporary_video() <<std:endl;
    cv_name = video.get_video_id();
    input_frame_count = video.get_frame_count();
    cv::VideoCapture vcl_written_video(video.get_cv_video());
    //input_frame_count = vcl_written_video.get_frame_count();
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
      std::cout<< "video frame count\t"<<video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_video\t"<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_written_vdeio\t"<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"vcl_vedio\t"<<(video.get_cv_video()).get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }

catch(VCL::Exception &e) {
        print_exception(e);
    }

}


TEST_F(VideoTest, Crop)
{

   try {
    std::cout<< "Crop Operation" << std::endl;
    VCL::Video video(_video); //
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.crop(VCL::Rectangle(0, 0, 50, 50));
    video.store(_video, VCL::Video::Format::XVID);
    testVideo = cv::VideoCapture("videos/Megamind.avi");
    long test_frame_count;
    long input_frame_count;
    std::string test_vi_id="videos_test/test_video_crop.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
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
    cv::VideoCapture test_written_video(test_vi_id);
   
    count =10;


        while ( count < _stop ) {
        cv::Mat frame;
        if ( test_written_video.read(frame) ) {
          std::cout<<count<<std::endl;
            
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;

    }
    std::string cv_name ;
    cv_name = video.get_video_id();
    std::cout<<cv_name<<std::endl;
    cv::VideoCapture vcl_written_video(video.get_cv_video());
    input_frame_count = video.get_frame_count();
    test_frame_count = test_written_video.get(CV_CAP_PROP_FRAME_COUNT);
     std::cout<< "video frame count\t"<<video.get_frame_count()<<"\t"<<vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_video\t"<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"test_written_vdeio\t"<<test_written_video.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    std::cout<<"vcl_vedio\t"<<(video.get_cv_video()).get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, test_frame_count);
  }

catch(VCL::Exception &e) {
        print_exception(e);
    }


}