/**
 * @file   VideoData.h
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
 * This file declares the C++ API for VideoData. VideoData contains all of the
 *  messy details about the Video that aren't visible in Video.h/Video.cc. It
 *  keeps track of which operations to perform (and in what order), as well as
 *  the raw data if it is in a CV Mat, and a pointer to the TDB raw data if it is
 *  in TDB format.
 */

#pragma once

#include <memory>

#include "Video.h"
// #include "Image.h"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace VCL {

    class VideoData {

    /*  *********************** */
    /*        OPERATION         */
    /*  *********************** */
        enum OperationType { READ, WRITE, RESIZE, CROP, THRESHOLD, INTERVAL };

        /**
         *  Provides a way to keep track of what operations should
         *   be performed on the data when it is needed
         *
         *  Operation is the base class, it keeps track of the format
         *   of the Video data, defines a way to convert Format to
         *   a string, and defines a virtual function that overloads the
         *   () operator
         */
        class Operation {

        protected:
            /** The format of the Video for this operation */
            VCL::Video::Format _format;

            /**
             *  Constructor, sets the format
             *
             *  @param format  The format for the operation
             *  @see Video.h for more details on Format
             */
            Operation(Video::Format format)
                : _format(format)
            {
            };

        public:
            /**
             *  Implemented by the specific operation, performs what
             *    the operation is supposed to do
             *
             *  @param img  A pointer to the current VideoData object
             */
            virtual void operator()(VideoData *video) = 0;

            virtual OperationType get_type() = 0;
        };

    /*  *********************** */
    /*       READ OPERATION     */
    /*  *********************** */
        /**
         *  Extends Operation, reads Video from the file system
         */
        class Read : public Operation {
        private:
            /** The full path to the object to read */
            std::string _fullpath;
            int _start; // specify the starting reading point of the video
            int _stop;  //specify the ending point of the reading it represnet sa frame index
            int _step;  // specifiy the number of the skipped frames in the reading operation

        public:
            /**
             *  Constructor, sets the format and path for reading
             *
             *  @param filename  The full path to read from
             *  @param format  The format to read the Video from
             *  @see Video.h for more details on Format
             */
            Read(const std::string& filename, Video::Format format);

            /**
             *  Reads an Video from the file system (based on the format
             *    and file path indicated)
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);


            OperationType get_type() { return READ; };
        };

    /*  *********************** */
    /*       WRITE OPERATION    */
    /*  *********************** */
        /**
         *  Extends Operation, writes to the file system in the specified
         *    format
         */
        class Write : public Operation {
        private:
            /** The full path of where to write the Video */
            std::string _fullpath;
            /** The format the Video used to be stored as */
            Video::Format _old_format;
            /** Whether to store the metadata */

            int _start;
            int _stop;
            int _step;


        public:
            /**
             *  Constructor, sets the formats and path for writing
             *
             *  @param filename  The full path to write to
             *  @param format  The format to store the Video in
             *  @param old_format  The format the Video was stored in
             *  @see Video.h for more details on Format
             */
            Write(const std::string& filename, Video::Format format,
                Video::Format old_format);

            /**
             *  Writes an Video to the file system (based on the format
             *    and file path indicated)
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);

            OperationType get_type() { return WRITE; };
        };

    /*  *********************** */
    /*       RESIZE OPERATION   */
    /*  *********************** */
        /**
         *  Extends Operation, resizes the Video to the specified size
         */
         class Resize : public Operation {
         private:
            /** Gives the height and width to resize the Video to */
            Rectangle _rect;
            int _start;
            int _stop;
            int _step;

        public:
            /**
             *  Constructor, sets the size to resize to and the format
             *
             *  @param rect  Contains height and width to resize to
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on Format and Rectangle
             */
            Resize(const Rectangle &rect, Video::Format format)
                : Operation(format),
                  _rect(rect)


            {
            };

            /**
             *  Resizes an Video to the given dimensions
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);

            OperationType get_type() { return RESIZE; };
        };


                    /*  *********************** */
                    /*      Interval Operation  */
                    /*  *********************** */
        class Interval : public Operation {
         private:
            /** Gives the height and width to resize the Video to */
          int _start;
          int _stop;
          int _step;

        public:
            /**
             *  Constructor, sets the size to resize to and the format
             *
             *  @param rect  Contains height and width to resize to
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on Format and Rectangle
             */
            Interval(const int start , const int stop, int step, Video::Format format)
                : Operation(format),
                  _start(start),
                  _stop(stop),
                  _step(step)
            {
            };

            /**
             *  Resizes an Video to the given dimensions
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);

            OperationType get_type() { return INTERVAL; };
        };

    /*  *********************** */
    /*       CROP OPERATION     */
    /*  *********************** */
        /**
         *  Extends Operation, crops the Video to the specified area
         */
         class Crop : public Operation {
         private:
            /** Gives the dimensions and coordinates of the desired area */
            Rectangle _rect;
            int _start;
            int _stop;
            int _step;

        public:
            /**
             *  Constructor, sets the area to crop to and the format
             *
             *  @param rect  Contains dimensions and coordinates of
             *    desired area
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on Video::Format and Rectangle
             */
            Crop(const Rectangle &rect, Video::Format format )
                : Operation(format),
                  _rect(rect)

            {
            };

            /**
             *  Crops the Video to the given area
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);

            OperationType get_type() { return CROP; };
        };

    /*  *********************** */
    /*    THRESHOLD OPERATION   */
    /*  *********************** */
        /**  Extends Operation, performs a thresholding operation that
         *     discards the pixel value if it is less than or equal to the
         *     threshold and sets that pixel to 0
         */
        class Threshold : public Operation {
        private:
            /** Minimum value pixels should be */
            int _threshold;
            int _start;
            int _stop;
            int _step;

        public:
            /**
             *  Constructor, sets the threshold value and format
             *
             *  @param value  Minimum value pixels should be
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on Video::Format
             */
            Threshold(const int value, Video::Format format)
                : Operation(format),
                  _threshold(value)

            {
            };

            /**
             *  Performs the thresholding operation
             *
             *  @param img  A pointer to the current VideoData object
             */
            void operator()(VideoData *video);

            OperationType get_type() { return THRESHOLD; };
        };


    private:
    /*  *********************** */
    /*        VARIABLES         */
    /*  *********************** */
        // Video height and width


    VideoData* _video;

    std::string _video_id;

    cv::VideoCapture _cv_video;

    cv::VideoCapture _inputVideo;

    cv::VideoWriter _outputVideo;

    std::string _temporary_path;
    std::string _temporary_video;
    bool _temp_exist = false;


    int  _frame_count;
    float _fps;
    unsigned char* _encoded_video;
    long _encoded_size;
    std::string _video_unit;
    uint _length;

    int _scale_num = 8 ;
    const float scale_stride = sqrt(3) ;

    uint _frame_width;
    uint _frame_height;
    int _start_frame = 0;
    int _end_frame = INT_MAX;
    int _step = 1;
    cv::Size _size;
    int _video_time ;

    VCL::Video::Format _format;
    Video::CompressionType _compress;
    int _cv_type;

    std::vector<std::shared_ptr<Operation>> _operations;



    public:


    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
        /**
         *  Default constructor, creates an empty VideoData object.
         *    Used when reading from the file system
         */
        VideoData();

               /**
         *  Creates an VideoData object from the filename
         *
         *  @param Video_id  A string indicating where the Video is on disk
         */
        VideoData(const std::string &video_id);

        VideoData(const cv::VideoCapture &cv_video );


       /**
         *  Creates an VideoData object from the given parameters
         *
         *  @param buffer  An buffer that contains the Video data in raw pixels
         *  @param dimensions  An OpenCV Size object that contains the height
         *    and width of the Video
         *  @param type  The OpenCV type of the Video
         *  @see OpenCV documentation for more information on type and Size
         */
        //VideoData(void* buffer, cv::Size dimensions);

        /**
         *  Creates an VideoData object from an existing VideoData object
         *
         *  @param img  A reference to an existing VideoData object
         */
        VideoData(const VideoData &video);

         // creates a video from an encoded buffer
        VideoData(void*  buffer, long size);

        /**
         *  Sets an VideoData object equal to another VideoData object
         *
         *  @param img  A reference to an existing VideoData object
         */
        void operator=(const VideoData &video);

        ~VideoData();


    /*  *********************** */
    /*        GET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Gets the Video id of the VideoData object
         *
         *  @return The string containing the full path to the VideoData object
         */
        std::string get_video_id() const;
        cv::VideoCapture get_cv_video() const;

        long get_frame_count(void) const;

          int default_ending = get_frame_count();

          cv::VideoWriter get_output_video(void);

        /**
         *  Gets the format of the VideoData object
         *
         *  @return The Format of the VideoData object
         *  @see Video.h for more details on Format
         */
        VCL::Video::Format get_video_format() const;

        /**
         *  Gets the OpenCV type of the Video
         *
         *  @return The OpenCV type (CV_8UC3, etc)
         *  @see OpenCV documentation on types for more details
         */
        int get_type() const;

        /**
         *  Gets the dimensions (height and width) of the Video
         *
         *  @return The height and width of the Video as an OpenCV Size object
         */
        cv::Size get_dimensions();

        /**
         *  Gets the size of the Video in pixels (height * width * channels)
         *
         *  @return The size of the Video in pixels
         */
        cv::Size get_size();




        /**
         *  Gets the Video data in a buffer
         *
         *  @param  buffer  A buffer (of any type) that will contain the Video
         *     data when the function ends
         *  @param  buffer_size  The pixel size of the Video (length of
         *     the buffer, not bytes)
         */
        void get_buffer(void* buffer, int buffer_size);

        /**
         *  Gets an OpenCV Mat that contains the Video data
         *
         *  @return An OpenCV Mat
         */
        cv::VideoCapture get_cv_video_capture();
      /**
         *  Gets a specific area of the Video, indicated by the Rectangle
         *    parameters
         *
         *  @param roi  The region of interest (starting x coordinate, starting
         *    y coordinate, height, width)
         *  @return VideoData of the area
         *  @see Video.h for more details about Rectangle
         */
        VideoData get_area(const Rectangle &roi);

        /**
         *  Gets encoded Video data in a buffer
         *
         *  @param format  The VCL::Video::Format the Video should be encoded as
         *  @param buffer  The buffer the encoded Video will be stored in
         *  @param params  Optional parameters
         *  @see OpenCV documentation for imencode for more details
         */
        char*  get_encoded(VCL::Video::Format format,
                                     const std::vector<int>& params=std::vector<int>());

        long get_size_encoded();


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Creates a unique ID in the path given with the given extension
         *
         *  @param path  A string with the path to where the Video should be
         *                  stored
         *  @param format The VCL::Video::Format the Video should be stored as
         *  @return The string containing the full path to the Video (path
         *    + unique id + format)
         */
        std::string create_unique(const std::string &path,
                VCL::Video::Format format);

         std::string format_to_string(VCL::Video::Format format);

        std::string remove_extention(const std::string path);

        /**
         *  Sets the file system location of where the Video
         *    can be found
         *
         *  @param Video_id  The full path to the Video location
         */
        void set_video_id(const std::string &Video_id);

        /**
         *  Sets the format of the VideoData object
         *
         *  @param extension  A string containing the file system
         *    extension corresponding to the desired VCL::Video::Format
         *  @see Video.h for more details on VCL::Video::Format
         */
        void set_format_from_extension(const std::string &extension);

        void set_format(int form);

        void set_temporary_directory(const std::string &path);

        /**
         *  Sets the type of the VideoData object using OpenCV types
         *
         *  @param cv_type  The OpenCV type of the object
         *  @see OpenCV documentation on types for more details
         */
        void set_type(int cv_type);

        /**
         *  Sets the type of compression to be used when compressing
         *    the TDBVideo
         *
         *  @param comp  The compression type
         *  @see Video.h for details on Video::CompressionType
         */
         void set_compression(Video::CompressionType comp) ;


        /**
         *  Sets the height and width of the Video
         *
         *  @param dimensions  The height and width of the Video
         *  @see OpenCV documentation for more details on Size
         */
        void set_dimensions(cv::Size dimensions);

        /**
         *  Sets the Video object to contain raw pixel data
         *    from a buffer of raw pixel data (stored in a TDB object)
         *
         *  @param buffer  The buffer containing the raw pixel data
         *  @param size  The size of the buffer
         */
        void set_data_from_raw(void* buffer, int size);

        /**
         *  Sets the Video object to contain raw pixel data
         *    from an encoded Video buffer (stored in a CV Mat)
         *
         *  @param buffer  The buffer containing the encoded pixel data
         */


    /*  *********************** */
    /*   VideoDATA INTERACTION  */
    /*  *********************** */
        /**
         *  Performs the set of operations that have been requested
         *    on the VideoData
         */
        void perform_operations();

        /**
         *  Stores a Read Operation in the list of operations
         *    to perform
         *
         *  @param Video_id  The full path to the Video to be read
         */
        void read(const std::string &video_id );

        /**
         *  Stores a Write Operation in the list of operations
         *    to perform
         *
         *  @param Video_id  The full path to where the Video should
         *    be written
         *  @param Video_format  The VCL::Video::Format to write the Video in
         *  @param store_metadata  A flag to indicate whether to store the
         *    metadata in TileDB or not. Defaults to true
         *  @see Video.h for more details on VCL::Video::Format
         */
        void write(const std::string &video_id,  VCL::Video::Format video_format);

        // void remove(const std::string &Video_id);

        /**
         *  Stores a Resize Operation in the list of operations
         *    to perform
         *
         *  @param rows  The number of rows in the resized Video
         *  @param columns  The number of columns in the resized Video
         */
        void resize(int rows, int columns);

        void interval(std::string unit, int start, int stop, int step);

        /**
         *  Stores a Crop Operation in the list of operations
         *    to perform
         *
         *  @param rect  The region of interest (starting x coordinate,
         *    starting y coordinate, height, width) the Video should be
         *    cropped to
         *  @see Video.h for more details about Rectangle
         */
        void crop(const Rectangle &rect);

        /**
         *  Stores a Threshold Operation in the list of operations
         *    to perform
         *
         *  @param value  The threshold value
         */
        void threshold(int value);

        /**
         *  Deletes the VideoData as well as removes file from system if
         *    it exists
         */
        void delete_object();



    private:
    /*  *********************** */
    /*      COPY FUNCTIONS      */
    /*  *********************** */
        /**
         *  Copies an OpenCV Mat into the VideoData OpenCV Mat
         *
         *  @param cv_img  An existing OpenCV Mat
         */
        void copy_cv(const cv::VideoCapture &cv_video);



    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */

        /**
         *  Creates full path to Video with appropriate extension based
         *    on the VCL::Video::Format
         *
         *  @param filename The path to the Video object
         *  @param format  The VCL::Video::Format of the Video object
         *  @return Full path to the object including extension
         */
        std::string create_fullpath(const std::string &filename,
            VCL::Video::Format format);
    };

}
