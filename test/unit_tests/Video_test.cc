#include "gtest/gtest.h"

#include <opencv2/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <string>

#include "Video.h"


class VideoTest : public ::testing::Test {
protected:
    virtual void SetUp() {

		filename = "/home/ragaad/newVCL/vcl/test/videos/pedestrian1.avi";
    }


    std::string filename;
};

TEST_F(VideoTest, DefaultConstructor)
{
    //VCL::Video video(filename);
    // video.read_video();
    // std::cout<< video.get_video();

    std::cout << " Video Test  " << filename << std::endl;

    VCL::Video video;

    // cv::Size dims = img_data.get_dimensions();

    // EXPECT_EQ(0, dims.height);
    // EXPECT_EQ(0, dims.width);
}

TEST_F(VideoTest, StringConstructor)
{
    //VCL::Video video(filename);
    // video.read_video();
    // std::cout<< video.get_video();

    std::cout << " Video Test  " << filename << std::endl;

    VCL::Video video(filename);

    // cv::Size dims = img_data.get_dimensions();

    // EXPECT_EQ(0, dims.height);
    // EXPECT_EQ(0, dims.width);
}

