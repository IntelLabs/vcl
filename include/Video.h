/**
 * @file   Video.h
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
 * @section DESCRIPTION
 *
 */

#pragma once

#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>


#include "Exception.h"
#include "utils.h"

//#include "VideoUtils.h"


namespace VCL {
    class VideoData; // To hide the Video implementation details


    typedef cv::Rect Rectangle; // spcifiy an ROI inside a video

    class Video {

    private:

    VideoData *_video; // Pointer to a VideoData object




    public:
    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
        /**
         *  Creates an Video object from the video id (where the
         *    video data can be found in the system)
         *
         *  @param video_id  The full path to the video
         */
        Video();
        Video(const std::string &fileName); // fileName is the full path to the video

        Video(const cv::VideoCapture video);

        Video(void* buffer, int size, const std::string &path = "tests/db/tmp"); // creates a video from an encoded buffer

         Video ( const Video &video);

        void operator=(const Video &video);



        ~Video();

    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */

        std::string get_video_id() const;

        cv::Size get_dimensions() const; //instead of get dimension

        Format get_video_format() const;

        int get_video_type() const;


        int get_raw_dat_size() const;

        void get_raw_data(void* buffer, long buffer_size) const;

        char* get_encoded_video(VCL::Format format,
                const std::vector<int>& params=std::vector<int>()) const;

        long get_encoded_size();

        std::string create_unique(const std::string &path,
                Format format);
        /**
         *  Sets the size of the image in pixels (width, height) using
         *    an OpenCV Size object
         *
         *  @param dims  The dimensions of the image in OpenCV Size format
         *  @see OpenCV documentation on Size for more details
         */
        void set_dimensions(cv::Size dims);

        /**
         *  Sets the type of compression to be used when compressing. Currently
         *    applicable only to TileDB
         *
         *  @param comp  The compression type
         */
        void set_compression(CompressionType comp);


        /**
         *  Sets the OpenCV type of the image
         *
         *  @param The OpenCV type (CV_8UC3, etc)
         *  @see OpenCV documentation on types for more details
         */
        void set_video_type(int cv_type);

        void set_minimum_dimension(int dimension);

    /*  *********************** */
    /*    IMAGE INTERACTIONS    */
    /*  *********************** */
        /**
         *  Writes the Image to the system at the given location and in
         *    the given format
         *
         *  @param image_id  Full path to where the image should be written
         *  @param image_format  Format in which to write the image
         *  @param store_metadata  Flag to indicate whether to store the
         *    image metadata. Defaults to true (assuming no other metadata
         *    storage)
         */
        void store(const std::string &video_id, Format video_format,
            bool store_metadata=true);

        /**
         *  Deletes the Video
         */
        void delete_video();

        /**
         *  Resizes the Video to the given size. This operation is not
         *    performed until the data is needed (ie, store is called or
         *    one of the get_ functions such as get_cvmat)
         *
         *  @param new_height  Number of rows
         *  @param new_width  Number of columns
          * start is the starting time
          * stop is the stopping time
          * step is the step length
         */
        void resize(int new_height, int new_width);

        /**
         *  Crops the Video to the area specified. This operation is not
         *    performed until the data is needed (ie, store is called or
         *    one of the get_ functions such as get_cvmat)
         *
         *  @param rect  The region of interest (starting x coordinate,
         *    starting y coordinate, height, width) the image should be
         *    cropped to
         */
        void crop(const Rectangle &rect);

        /**
         *  Performs a thresholding operation on the Video. Discards the pixel
         *    value if it is less than or equal to the threshold and sets that
         *    pixel to zero. This operation is not performed until the data
         *    is needed (ie, store is called or one of the get_ functions
         *    such as get_cvmat)
         *
         *  @param value  The threshold value
         */
        void threshold(int value);

        void interval (std::string unit, int start, int stop, int step=1);
        long get_frame_count(void) const;




    };

};
