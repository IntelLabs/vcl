#include "gtest/gtest.h"

#include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>


#include <string>
#include <iostream>
#include <fstream>
#include "Video.h"


class VideoTest : public ::testing::Test {
    protected:
    virtual void SetUp() {
        _video = "test/videos/pedestrian1.avi";
        _inputVideo = cv::VideoCapture("test/videos/pedestrian1.avi");
    }


    std::string _video;
    cv::VideoCapture _inputVideo;


};

TEST_F(VideoTest, DefaultConstructor)
{
    //VCL::Video video(filename);
    // video.read_video();s
    // std::cout<< video.get_video();
try{
    std::cout<<"empty video";
    VCL::Video video();
   //std::cout << video.get_frame_count();
}
catch(VCL::Exception &e) {
        print_exception(e);
    }

    // EXPECT_EQ(0, dims.height);
    // EXPECT_EQ(0, dims.width);
}

TEST_F(VideoTest, StringConstructor)
{
    //VCL::Video video(filename);
    // video.read_video();
    // std::cout<< video.get_video();

    //std::cout << " Video String Test  " << filename << std::endl;

    VCL::Video video(_video);

    // cv::Size dims = img_data.get_dimensions();

    // EXPECT_EQ(0, dims.height);
    // EXPECT_EQ(0, dims.width);
}

TEST_F(VideoTest, ObjectConstructor)
{
    std::cout<< " CVCapture Object Constructor" << std::endl;

    VCL::Video video_data(_inputVideo); //
   // std::cout<< " The Video Size is " << video_data.get_size();


}

TEST_F(VideoTest, BlobConstructor)
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

    VCL::Video video_data(inBuf, fsize, "test/temp/"); //
    std::cout<< " The Video Size is " << fsize <<"\t"<< sizeof(inBuf)<<std::endl;
    delete[] inBuf;

}

/*{
   try {
    std::cout<< " Read Operation" << _video<< std::endl;

    VCL::Video video(_video);
    video.read(_video, 100, 200, 1);

}
catch(VCL::Exception &e) {
        print_exception(e);
    }
    // std::cout<< " The Video Size is " << video.get_size();


 }*/

// TEST_F(VideoTest, Store)
// {
//     try{
//     std::cout<< " Store Operation" << std::endl;

//     VCL::Video video(_video); //
//     video.store(_video, VCL::Format::XVID, true, 10, 200, 10);

// }
// catch(VCL::Exception &e) {
//         print_exception(e);
//     }

// }
/*
 TEST_F(VideoTest, Resize)
{
    try{
    std::cout<< "Resize Operation" << std::endl;

   VCL::Video video(_inputVideo);

   video.resize( 100,100, 10, 2000, 2);


   }
    catch(VCL::Exception &e) {
        print_exception(e);
    }

}

TEST_F(VideoTest, Threshold)
{
   try{
    std::cout<< "threshold Operation" << std::endl;

   VCL::Video video(_inputVideo);

    video.threshold( 100, 50, 1000,10);


}
catch(VCL::Exception &e) {
        print_exception(e);
    }

}

TEST_F(VideoTest, CreateUnique)
{
     try{
     VCL::Video video(_inputVideo); //

    video.create_unique("test/temp/", VCL::Format::MP4);
    std::cout<< video.get_video_id() <<std::endl;
    video.store(video.get_video_id(), VCL::Format::MP4, true, 0, 100, 1);
    }
catch(VCL::Exception &e) {
        print_exception(e);
    }


}

TEST_F(VideoTest, Crop)
{
    try {
    VCL::Video video(_inputVideo);


    video.crop(VCL::Rectangle(0, 0, 50, 50), 100, 1500,10);

    }
    catch(VCL::Exception &e) {
        print_exception(e);
    }
}*/