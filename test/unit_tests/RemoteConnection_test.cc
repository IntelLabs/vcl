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

#ifdef S3_SUPPORT

#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

#include "TDBImage.h"
#include "ImageData.h"
#include "Image.h"
#include "gtest/gtest.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <string>


class RemoteConnectionTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
		Aws::Utils::Logging::InitializeAWSLogging(
    		Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
        	"RunUnitTests", Aws::Utils::Logging::LogLevel::Trace, "aws_sdk_"));

        img_ = "images/large1.jpg";
        tdb_img_ = "tdb/test_image.tdb";
        cv_img_ = cv::imread(img_, cv::IMREAD_ANYCOLOR);
        rect_ = VCL::Rectangle(100, 100, 100, 100);
    
        // _remotepath = "s3://<path/to/your/bucket>";
        // connection_ = new VCL::RemoteConnection();
        // connection_->set_s3_configuration("us-west-2");
        // connection_->set_s3_proxy(<PROXY_HOST>, <PROXY_PORT>);
        // connection_->start();
        // connection_->set_s3_credentials(<ACCESS_ID>, <SECRET_KEY>);
    }

    virtual void TearDown() {
        connection_->end();
        delete connection_;
        Aws::Utils::Logging::ShutdownAWSLogging();
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

    std::string img_;
    std::string tdb_img_;
    std::string test_img_;
    cv::Mat cv_img_;
    VCL::Rectangle rect_;

    std::string _remotepath;
    VCL::RemoteConnection *connection_;
};

// TEST_F(RemoteConnectionTest, ReadS3)
// {
//     try {
//     VCL::Image img(_remotepath, *connection_);

//     cv::Mat mat = img.get_cvmat();

//     VCL::Image tdb(mat);

//     tdb.store("freedtest", VCL::TDB);
//     }
//     catch(VCL::Exception &e) {
//         print_exception(e);
//     }
// }

// TEST_F(RemoteConnectionTest, TDBImageWriteS3)
// {
//     VCL::TDBImage tdb(_remotepath + "tdb/test_image.tdb", *connection_);

//     tdb.write(cv_img_);
// }

// TEST_F(RemoteConnectionTest, TDBImageReadS3)
// {
//     std::string fullpath = _remotepath + "tdb/test_image.tdb";

//     VCL::TDBImage tdb(fullpath, *connection_);

//     int size = tdb.get_image_size();

//     unsigned char* buf = new unsigned char[size];

//     tdb.get_buffer(buf, size);

//     compare_mat_buffer(cv_img_, buf);

//     delete [] buf;
// }

// TEST_F(RemoteConnectionTest, TDBImageRemoveS3)
// {
//     VCL::TDBImage tdb(_remotepath + "tdb/test_image.tdb", *connection_);

//     tdb.delete_image();

//     ASSERT_FALSE(tdb.has_data());
//     ASSERT_THROW(tdb.get_image_size(), VCL::Exception);
// }

// TEST_F(RemoteConnectionTest, ImageDataRemoteWritePNG)
// {
//     VCL::ImageData img_data(cv_img_);

//     img_data.set_connection(*connection_);

//     std::string path = _remotepath + "pngs/test_image.png";

//     img_data.write(path, VCL::PNG);
//     img_data.perform_operations();
// }

// TEST_F(RemoteConnectionTest, ImageDataRemoteReadPNG)
// {
//     VCL::ImageData img_data;

//     img_data.set_connection(*connection_);

//     std::string path = _remotepath + "pngs/test_image.jpg";

//     img_data.read(_remote_path);

//     cv::Mat data = img_data.get_cvmat();

//     compare_mat_mat(data, cv_img_);
// }

// TEST_F(RemoteConnectionTest, ImageDataRemoteWriteTDB)
// {
//     VCL::ImageData img_data(cv_img_);

//     img_data.set_connection(*connection_);

//     img_data.write(_remotepath + "tdb/test_image", VCL::TDB);
//     img_data.perform_operations();
// }

// TEST_F(RemoteConnectionTest, ImageDataRemoteReadTDB)
// {
//     VCL::ImageData img_data(_remotepath + "tdb/test_image.tdb", *connection_);

//     img_data.read(_remotepath + "tdb/test_image.tdb");
        
//     cv::Mat data = img_data.get_cvmat();
//     compare_mat_mat(data, cv_img_);
// }

// TEST_F(RemoteConnectionTest, ImageRemoteWritePNG)
// {
//     VCL::Image img(cv_img_);

//     img.set_connection(*connection_);

//     std::string path = _remotepath + "pngs/test_image.png";

//     img.store(path, VCL::PNG);
// }

// TEST_F(RemoteConnectionTest, ImageRemoteReadJPG)
// {
//     VCL::Image img(_remotepath + "large1.jpg", *connection_);

//     cv::Mat mat = img.get_cvmat();

//     compare_mat_mat(mat, cv_img_);
// }

// TEST_F(RemoteConnectionTest, ImageRemoteReadTDB)
// {
//     VCL::Image img(_remotepath + "tdb/test_image.tdb", *connection_);

//     cv::Mat mat = img.get_cvmat();
//     compare_mat_mat(mat, cv_img_);   
// }

// TEST_F(RemoteConnectionTest, ImageRemoteRemoveTDB)
// {
//     VCL::Image img(_remotepath + "tdb/test_image.tdb", *connection_);
//     img.delete_image();
// }

// TEST_F(RemoteConnectionTest, ImageRemoteRemovePNG)
// {
//     VCL::Image img(_remotepath + "pngs/test_image.png", *connection_);
//     img.delete_image();
// }

#endif
