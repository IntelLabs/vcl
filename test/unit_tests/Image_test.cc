#include "Image.h"
#include "gtest/gtest.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>

class ImageTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        img_ = "images/large1.jpg";
        tdb_img_ = "tdb/images/test_image.tdb";
        cv_img_ = cv::imread(img_, -1);

        size_ = cv_img_.rows * cv_img_.cols * cv_img_.channels();
        rect_ = VCL::Rectangle(100, 100, 100, 100);
        bad_rect_ = VCL::Rectangle(1000, 1000, 10000, 10000);
        dimension_ = 256;
    }

    void compare_mat_buffer(cv::Mat &img, unsigned char* buffer)
    {
        int index = 0;

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
                    ASSERT_EQ(pixel, buffer[index]);
                }
                else {
                    cv::Vec3b colors = img.at<cv::Vec3b>(i, j);
                    for ( int x = 0; x < channels; ++x ) {
                        ASSERT_EQ(colors.val[x], buffer[index + x]);
                    }
                }
                index += channels;
            }
        }
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

    VCL::Rectangle rect_;
    VCL::Rectangle bad_rect_;
    std::string img_;
    std::string tdb_img_;

    cv::Mat cv_img_;

    int dimension_;
    int size_;
};


// When setting from a filename, we set the type, number of channels, path, and format of the image,
// We also add a read operation to the list of operations
TEST_F(ImageTest, StringConstructor)
{
    VCL::Image img(img_);

    EXPECT_EQ(VCL::JPG, img.get_image_format());
    EXPECT_EQ(img_, img.get_image_id());
}

// When setting from a cv::mat, we set the type of the image and copy the image data
// We should know the height, width, number of channels, type, and have a non-empty Mat
TEST_F(ImageTest, MatConstructor)
{
    VCL::Image img(cv_img_);

    EXPECT_EQ(cv_img_.type(), img.get_image_type());

    cv::Size dims = img.get_dimensions();

    EXPECT_EQ(cv_img_.rows, dims.height);
    EXPECT_EQ(cv_img_.cols, dims.width);

    ASSERT_FALSE( img.get_cvmat().empty() );
}

TEST_F(ImageTest, EncodedBufferConstructor)
{
    std::fstream jpgimage("images/large1.jpg");

    jpgimage.seekg(0, jpgimage.end);
    int length = jpgimage.tellg();
    jpgimage.seekg(0, jpgimage.beg);

    char* buffer = new char[length];
    jpgimage.read(buffer, length);
    jpgimage.close();

    int size = cv_img_.rows * cv_img_.cols * cv_img_.channels();

    VCL::Image img(buffer, size);

    ASSERT_FALSE(img.get_cvmat().empty());
    cv::Mat raw = img.get_cvmat();

    compare_mat_mat(cv_img_, raw);
}

TEST_F(ImageTest, RawBufferConstructor)
{
    void* buffer = cv_img_.data;

    VCL::Image img(buffer, cv::Size(cv_img_.cols, cv_img_.rows), cv_img_.type());

    cv::Mat raw = img.get_cvmat();

    compare_mat_mat(cv_img_, raw);
}

TEST_F(ImageTest, CopyConstructor)
{
    VCL::Image img(cv_img_);

    EXPECT_EQ(cv_img_.type(), img.get_image_type());

    VCL::Image test_img(img);

    EXPECT_EQ(cv_img_.type(), test_img.get_image_type());

    cv::Mat test_cv = test_img.get_cvmat();
    ASSERT_FALSE( test_cv.empty() );

    compare_mat_mat(test_cv, cv_img_);
}

TEST_F(ImageTest, CopyConstructorComplex)
{
    VCL::Image img(cv_img_);

    EXPECT_EQ(cv_img_.type(), img.get_image_type());

    img.crop(rect_);

    VCL::Image test_img(img);

    EXPECT_EQ(cv_img_.type(), test_img.get_image_type());
    cv::Mat test_cv = test_img.get_cvmat();

    cv::Mat cropped_cv(cv_img_, rect_);
    compare_mat_mat(test_cv, cropped_cv);

    cv::Size dims = test_img.get_dimensions();
    EXPECT_EQ(rect_.height, dims.height);
}


TEST_F(ImageTest, GetMatFromMat)
{
    VCL::Image img(cv_img_);

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());

    compare_mat_mat(cv_img, cv_img_);
}

TEST_F(ImageTest, GetMatFromPNG)
{
    VCL::Image img(img_);

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());

    compare_mat_mat(cv_img, cv_img_);
}

TEST_F(ImageTest, GetMatFromTDB)
{
    VCL::Image img(tdb_img_);

    EXPECT_EQ(tdb_img_, img.get_image_id());
    EXPECT_EQ(VCL::TDB, img.get_image_format());

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());

    compare_mat_mat(cv_img, cv_img_);
}

TEST_F(ImageTest, GetBufferFromMat)
{
    VCL::Image img(cv_img_);

    unsigned char* buffer = new unsigned char[img.get_raw_data_size()];

    img.get_raw_data(buffer, img.get_raw_data_size());

    EXPECT_TRUE(buffer != NULL);
    compare_mat_buffer(cv_img_, buffer);

    delete [] buffer;
}

TEST_F(ImageTest, GetBufferFromPNG)
{
    VCL::Image img(img_);

    unsigned char* buffer = new unsigned char[img.get_raw_data_size()];

    img.get_raw_data(buffer, img.get_raw_data_size());

    EXPECT_TRUE(buffer != NULL);
    compare_mat_buffer(cv_img_, buffer);

    delete [] buffer;
}

TEST_F(ImageTest, GetBufferFromTDB)
{
    VCL::Image img(tdb_img_);

    int size = img.get_raw_data_size();
    unsigned char* buffer = new unsigned char[size];

    img.get_raw_data(buffer, size);

    EXPECT_TRUE(buffer != NULL);
    compare_mat_buffer(cv_img_, (unsigned char*)buffer);

    delete [] buffer;
}

TEST_F(ImageTest, GetRectangleFromPNG)
{
    VCL::Image img(img_);

    VCL::Image corner = img.get_area(rect_);

    cv::Size dims = corner.get_dimensions();

    EXPECT_EQ(rect_.height, dims.height);
    EXPECT_EQ(rect_.width, dims.width);
}

TEST_F(ImageTest, GetRectangleFromTDB)
{
    VCL::Image img(tdb_img_);
    try{
    VCL::Image corner = img.get_area(rect_);
    cv::Size dims = corner.get_dimensions();

    EXPECT_EQ(rect_.height, dims.height);
    EXPECT_EQ(rect_.width, dims.width);
    } catch(VCL::Exception &e) {
    print_exception(e);
    }
}

TEST_F(ImageTest, GetRectangleFromMat)
{
    VCL::Image img(cv_img_);

    VCL::Image corner = img.get_area(rect_);

    cv::Size dims = corner.get_dimensions();

    EXPECT_EQ(rect_.height, dims.height);
    EXPECT_EQ(rect_.width, dims.width);
}


TEST_F(ImageTest, WriteMatToJPG)
{
    VCL::Image img(cv_img_);
    img.store("image_results/test_image", VCL::JPG);

    cv::Mat test = cv::imread("image_results/test_image.jpg");

    EXPECT_FALSE( test.empty() );
}

TEST_F(ImageTest, WriteMatToTDB)
{
    VCL::Image img(cv_img_);
    img.store("tdb/images/mat_to_tdb", VCL::TDB);
}

TEST_F(ImageTest, WriteStringToTDB)
{
    VCL::Image img(img_);
    img.store("tdb/images/png_to_tdb.png", VCL::TDB);
}

TEST_F(ImageTest, ResizeMat)
{
    VCL::Image img(img_);
    img.resize(dimension_, dimension_);

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());
    EXPECT_EQ(dimension_, cv_img.rows);
}

TEST_F(ImageTest, ResizeTDB)
{
    VCL::Image img(tdb_img_);
    img.resize(dimension_, dimension_);

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());
    EXPECT_EQ(dimension_, cv_img.rows);
}

TEST_F(ImageTest, CropMatThrow)
{
    VCL::Image img(img_);
    img.crop(bad_rect_);

    ASSERT_THROW(img.get_cvmat(), VCL::Exception);
}

TEST_F(ImageTest, CropMat)
{
    VCL::Image img(img_);
    img.crop(rect_);

    cv::Mat cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());
    EXPECT_EQ(rect_.height, cv_img.rows);
}

TEST_F(ImageTest, TDBMatThrow)
{
    VCL::Image img(tdb_img_);
    img.crop(bad_rect_);

    ASSERT_THROW(img.get_cvmat(), VCL::Exception);
}

TEST_F(ImageTest, CropTDB)
{
    cv::Mat cv_img;

    VCL::Image img(tdb_img_);

    img.crop(rect_);

    cv_img = img.get_cvmat();

    EXPECT_FALSE(cv_img.empty());
    EXPECT_EQ(rect_.height, cv_img.rows);
}

TEST_F(ImageTest, CompareMatAndBuffer)
{
    VCL::Image img(img_);

    unsigned char* data_buffer = new unsigned char[img.get_raw_data_size()];
    img.get_raw_data(data_buffer, img.get_raw_data_size());

    compare_mat_buffer(cv_img_, data_buffer);
}

TEST_F(ImageTest, TDBToPNG)
{
    VCL::Image img(tdb_img_);

    img.store("image_results/tdb_to_png", VCL::PNG);
}

TEST_F(ImageTest, TDBToJPG)
{
    VCL::Image img(tdb_img_);

    img.store("image_results/tdb_to_jpg", VCL::JPG);
}

TEST_F(ImageTest, EncodedImage)
{
    VCL::Image img(tdb_img_);

    std::vector<unsigned char> buffer = img.get_encoded_image(VCL::PNG);

    cv::Mat mat = cv::imdecode(buffer, cv::IMREAD_ANYCOLOR);
    compare_mat_mat(cv_img_, mat);
}

TEST_F(ImageTest, CreateName)
{
    VCL::Image img(cv_img_);

    for ( int i = 0; i < 10; ++i ) {
        std::string name = img.create_unique("tdb/images/", VCL::TDB);
        img.store(name, VCL::TDB);
    }
}

TEST_F(ImageTest, NoMetadata){
    VCL::Image img(cv_img_);

    std::string name = img.create_unique("tdb/images/", VCL::TDB);
    img.store(name, VCL::TDB, false);

    cv::Size dims = img.get_dimensions();
    int cv_type = img.get_image_type();

    VCL::Image tdbimg(name);

    tdbimg.set_image_type(cv_type);
    tdbimg.set_dimensions(dims);

    cv::Mat mat = tdbimg.get_cvmat();
}
