#include "gtest/gtest.h"

#include <string>
#include <iostream>
#include <fstream>
#include "Video.h"

#include "helpers.h"

class VideoTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        _video = "videos/Megamind.avi";
        _inputVideo = cv::VideoCapture(_video);

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
    VCL::Video::Video_Size vcl_size;
    VCL::Video::Video_Size test_size;
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
    VCL::Video video(_video);
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture(_video);
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
}

TEST_F(VideoTest, ObjectConstructor)
{
    VCL::Video video(_inputVideo); //
    long input_frame_count = video.get_frame_count();
    long test_frame_count;
    testVideo = cv::VideoCapture(_video);
    test_frame_count= testVideo.get(CV_CAP_PROP_FRAME_COUNT);

    // std::cout<<testVideo.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;

    // std::cout<<video.get_cv_video().get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;

    // std::cout<< input_frame_count <<"=="<< test_frame_count <<std::endl;
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
    testVideo = cv::VideoCapture(_video);
    test_frame_count = testVideo.get(CV_CAP_PROP_FRAME_COUNT);
    ASSERT_EQ(input_frame_count, test_frame_count);
    compare_cvcapture_cvcapture(video.get_cv_video(), testVideo, 1, test_frame_count);
}


TEST_F(VideoTest, Read)
{
    try
    {
        VCL::Video video(_video);
        video.read(_video);
        // testVideo = cv::VideoCapture("videos/Megamind.avi");
        testVideo = cv::VideoCapture(_video);

        long test_frame_count = testVideo.get(CV_CAP_PROP_FRAME_COUNT);
        long input_frame_count = video.get_frame_count();
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
        video.create_unique("videos_test/", VCL::Video::Format::AVI);
        std::cout << video.get_video_id() << std::endl;
        video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
        video.store(video.get_video_id(), VCL::Video::Format::AVI);
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
        video.create_unique("videos_test/", VCL::Video::Format::AVI);
        std::cout << video.get_video_id() << std::endl;
        //video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
        video.store(video.get_video_id(), VCL::Video::Format::AVI);

        testVideo = cv::VideoCapture(_video);
        long test_frame_count = testVideo.get(CV_CAP_PROP_FRAME_COUNT);
        long input_frame_count = video.get_frame_count();
        ASSERT_EQ(input_frame_count, test_frame_count);
        //compare_cvcapture_cvcapture(video.get_cv_video(), testVideo, 1, test_frame_count);
    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }
 }

TEST_F(VideoTest, ReadStoredVideo)
{
    try {
    std::cout<< " Read Stored Video" << std::endl;
    VCL::Video video(_video); //
    std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.store(name, VCL::Video::Format::AVI);

    std::string test_vi_id = video.get_video_id();
    std::cout<< test_vi_id <<"\t"<< name <<std::endl;
    cv::VideoCapture vcl_written_video(test_vi_id);
    vcl_size = video.get_size();
    test_size.width = vcl_written_video.get(CV_CAP_PROP_FRAME_WIDTH);
    test_size.height = vcl_written_video.get(CV_CAP_PROP_FRAME_HEIGHT);
    test_size.frame_number = static_cast<long> (vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT));
    long input_frame_count = video.get_frame_count();

    std::cout<< vcl_size.width << "\t"<<vcl_size.height << "\t"<< vcl_size.frame_number <<std::endl;
    std::cout<< test_size.width << "\t"<<test_size.height << "\t"<< test_size.frame_number <<std::endl;
    long test_frame_count = static_cast<long> (vcl_written_video.get(CV_CAP_PROP_FRAME_COUNT));
    std::cout<< input_frame_count<<"\t"<<test_frame_count<<std::endl;
    if (vcl_size==test_size)
        std::cout <<"PASSED\n";


    // cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
    //                 CV_FOURCC('X', 'V', 'I', 'D'),
    //                 testVideo.get(CV_CAP_PROP_FPS),
    //                 cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 0;
    int _stop = video.get_frame_count();
    int _step = 1;
    while ( count < _stop ) {
        cv::Mat frame;
        if ( vcl_written_video.read(frame) ) {
            ASSERT_FALSE(frame.empty());
            // if ( !frame.empty() ) {
            //      testResultVideo.write(frame);
            //     std::cout<<"vcl frame being read\n";
            // }
            // else
            //     std::cout << "vcl frame empty\n";
        }
        else
            std::cout << "video not read\n";
        count+=_step;
    }
//    compare_cvcapture_cvcapture(video.get_cv_video(), vcl_written_video, 1, count);
    ASSERT_EQ(input_frame_count, count);


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
      std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.store(name, VCL::Video::Format::AVI);

    testVideo = cv::VideoCapture(_video);
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
    testVideo.release();
    testResultVideo.release();
    cv::VideoCapture test_written_video(test_vi_id);
    std::string cv_name ;

    cv::VideoCapture vcl_written_video(video.get_cv_video());
    input_frame_count = video.get_frame_count();
    ASSERT_EQ(input_frame_count, count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, count);
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
    std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.interval(VCL::Video::UNIT::FRAMES, 1, 200, 5);
    video.store(name, VCL::Video::Format::AVI);
    long test_frame_count;
    long input_frame_count;

    std::string test_vi_id = "videos_test/test_Video_interval.avi";
    testVideo = cv::VideoCapture(_video);
    cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    testVideo.get(CV_CAP_PROP_FPS),
                    cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 1;
    int _stop = 200;
    int _step = 5;
    int total_frames = 0;
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
        total_frames++;
    }
    testVideo.release();
    testResultVideo.release();

    std::string cv_name = video.get_video_id();
    cv::VideoCapture test_written_video(cv_name);
    cv::VideoCapture test_cv_video(test_vi_id);

    count = 0;
    _stop = video.get_frame_count();
    _step = 1;
    while ( count < _stop ) {
        cv::Mat frame1;
        cv::Mat frame2;
        if ( test_written_video.read(frame1) && test_cv_video.read(frame2)) {
            ASSERT_FALSE(frame1.empty());
            ASSERT_FALSE(frame2.empty());
            compare_mat_mat(frame1, frame2);
        }
        else {
            std::cout << "Test failed at frame " << count << std::endl;
            break;
        }
        count+=_step;
    }
    ASSERT_EQ(count, total_frames);

  }
    catch(VCL::Exception &e) {
        print_exception(e);
    }
}

TEST_F(VideoTest, Resize)
{
    try {
    std::cout<< " Resize Operation" << std::endl;
    VCL::Video video(_video);
    std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.interval(VCL::Video::UNIT::FRAMES,1, 200, 5);
    video.resize(10,10);
    video.store(name, VCL::Video::Format::AVI);

    testVideo = cv::VideoCapture(_video);

    long test_frame_count;
    long input_frame_count;

    std::string test_vi_id="videos_test/test_Video_resize.avi";
     cv::VideoWriter testResultVideo = cv::VideoWriter(test_vi_id,
                    CV_FOURCC('X', 'V', 'I', 'D'),
                    testVideo.get(CV_CAP_PROP_FPS),
                    cv::Size(testVideo.get(CV_CAP_PROP_FRAME_WIDTH),testVideo.get(CV_CAP_PROP_FRAME_HEIGHT)));
    int count= 1;
    int _stop = 200;
    int _step = 5;
    int total_frames = 0;
    while ( count < _stop ) {
        cv::Mat frame;
        if ( testVideo.read(frame) ) {
            if ( !frame.empty() ) {
                cv::Mat cv_resized;
                cv::resize(frame, cv_resized, cv::Size(10, 10));
                testResultVideo.write(cv_resized);
            }
            else
                throw VCLException(ObjectEmpty, "Frame is empty");
        }
        else
            throw VCLException(ObjectEmpty, "Frame not retrieved");
        count+=_step;
        total_frames++;
    }
    testVideo.release();
    testResultVideo.release();

    std::string cv_name = video.get_video_id();
    std::cout << "vcl name is " << cv_name << ", cv name is " << test_vi_id << std::endl;
    cv::VideoCapture test_written_video(cv_name);
    cv::VideoCapture test_cv_video(test_vi_id);

    count = 0;
    _stop = video.get_frame_count();
    _step = 1;
    while ( count < _stop ) {
        cv::Mat frame1;
        cv::Mat frame2;
        if ( test_written_video.read(frame1) && test_cv_video.read(frame2)) {
            ASSERT_FALSE(frame1.empty());
            ASSERT_FALSE(frame2.empty());
            compare_mat_mat(frame1, frame2);
        }
        else {
            std::cout << "Test failed at frame " << count << std::endl;
            break;
        }
        count+=_step;
    }
    ASSERT_EQ(count, total_frames);
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
    std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.threshold( 150);
    video.store(name, VCL::Video::Format::AVI);

    testVideo = cv::VideoCapture(_video);
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
    int total_frames = 0;
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
        total_frames++;
    }

    std::string cv_name = video.get_video_id();
    cv::VideoCapture test_written_video(cv_name);
    cv::VideoCapture test_cv_video(test_vi_id);

    count = 0;
    _stop = video.get_frame_count();
    _step = 1;
    while ( count < _stop ) {
        cv::Mat frame1;
        cv::Mat frame2;
        if ( test_written_video.read(frame1) && test_cv_video.read(frame2)) {
            ASSERT_FALSE(frame1.empty());
            ASSERT_FALSE(frame2.empty());
            //compare_mat_mat(frame1, frame2);
        }
        else {
            std::cout << "Test failed at frame " << count << std::endl;
            break;
        }
        count+=_step;
    }
    ASSERT_EQ(count, total_frames);
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
    std::string name = video.create_unique("videos_test/", VCL::Video::Format::AVI);
    video.interval(VCL::Video::UNIT::FRAMES,10, 200, 5);
    video.crop(VCL::Rectangle(0, 0, 50, 50));
     video.store(name, VCL::Video::Format::AVI);
    testVideo = cv::VideoCapture(_video);
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
    ASSERT_EQ(input_frame_count, count);
    compare_cvcapture_cvcapture(vcl_written_video, test_written_video, 1, count);
  }

catch(VCL::Exception &e) {
        print_exception(e);
    }


}