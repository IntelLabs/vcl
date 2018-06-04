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
         *   of the Video data, defines a way to convert VideoFormat to
         *   a string, and defines a virtual function that overloads the
         *   () operator
         */
        class Operation {

        protected:
            /** The format of the Video for this operation */
            VideoFormat _format;

            /**
             *  Constructor, sets the format
             *
             *  @param format  The format for the operation
             *  @see Video.h for more details on VideoFormat
             */
            Operation(VideoFormat format)
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

        public:
            /**
             *  Constructor, sets the format and path for reading
             *
             *  @param filename  The full path to read from
             *  @param format  The format to read the Video from
             *  @see Video.h for more details on VideoFormat
             */
            Read(const std::string& filename, VideoFormat format);

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
            VideoFormat _old_format;
            /** Whether to store the metadata */
            bool _metadata;

        public:
            /**
             *  Constructor, sets the formats and path for writing
             *
             *  @param filename  The full path to write to
             *  @param format  The format to store the Video in
             *  @param old_format  The format the Video was stored in
             *  @see Video.h for more details on VideoFormat
             */
            Write(const std::string& filename, VideoFormat format,
                VideoFormat old_format, bool metadata);
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

        public:
            /**
             *  Constructor, sets the size to resize to and the format
             *
             *  @param rect  Contains height and width to resize to
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on VideoFormat and Rectangle
             */
            Resize(const Rectangle &rect, VideoFormat format)
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
    /*       CROP OPERATION     */
    /*  *********************** */
        /**
         *  Extends Operation, crops the Video to the specified area
         */
         class Crop : public Operation {
         private:
            /** Gives the dimensions and coordinates of the desired area */
            Rectangle _rect;

        public:
            /**
             *  Constructor, sets the area to crop to and the format
             *
             *  @param rect  Contains dimensions and coordinates of
             *    desired area
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on VideoFormat and Rectangle
             */
            Crop(const Rectangle &rect, VideoFormat format)
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

        public:
            /**
             *  Constructor, sets the threshold value and format
             *
             *  @param value  Minimum value pixels should be
             *  @param format  The current format of the Video data
             *  @see Video.h for more details on VideoFormat
             */
            Threshold(const int value, VideoFormat format)
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

    std::string _file_name;
    std::string _path;

    int _start_frame = 0 ;
    int _end_frame = INT_MAX;
    int _scale_num = 8 ;
    const float scale_stride = sqrt(3) ;

    int _frame_width;
    int _frame_height;

    float _fps;
    float _frame_count;
    cv::Size _size;
    int _length;
    uint _height, _width;
    int _video_time;

        // Type of Video (OpenCV definition) and number of channels
        int _cv_type, _channels;

        // Maintains order of operations requested
        std::vector<std::shared_ptr<Operation>> _operations;

        // Video format and compression type
        VideoFormat _format;
        int _format1;


        // Full path to Video
        std::string _video_id;

        // Video data (OpenCV Mat or TDBVideo)
        cv::VideoCapture _inputVideo;
        cv::VideoWriter _outputVideo;



        std::ifstream* infile ;// (video_id,std::ifstream::binary);
        std::ofstream* outfile; // (video_id,std::ofstream::binary);




        // std::vector<Image> _frames;


    public:
    /*  *********************** */
    /*        CONSTRUCTORS      */
    /*  *********************** */
        /**
         *  Default constructor, creates an empty VideoData object.
         *    Used when reading from the file system
         */
        VideoData();

        // creates a video from an encoded buffer
        VideoData(void* buffer, int size);

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
        std::string get_Video_id() const;

        /**
         *  Gets the format of the VideoData object
         *
         *  @return The VideoFormat of the VideoData object
         *  @see Video.h for more details on VideoFormat
         */
        VideoFormat get_Video_format() const;

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
        cv::Mat get_cvmat();

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
         *  @param format  The VideoFormat the Video should be encoded as
         *  @param buffer  The buffer the encoded Video will be stored in
         *  @param params  Optional parameters
         *  @see OpenCV documentation for imencode for more details
         */
        std::vector<unsigned char> get_encoded(VideoFormat format,
            const std::vector<int>& params=std::vector<int>());


    /*  *********************** */
    /*        SET FUNCTIONS     */
    /*  *********************** */
        /**
         *  Creates a unique ID in the path given with the given extension
         *
         *  @param path  A string with the path to where the Video should be
         *                  stored
         *  @param format The VideoFormat the Video should be stored as
         *  @return The string containing the full path to the Video (path
         *    + unique id + format)
         */
        void create_unique(const std::string &path,
                VideoFormat format);

        /**
         *  Sets the file system location of where the Video
         *    can be found
         *
         *  @param Video_id  The full path to the Video location
         */
        void set_Video_id(const std::string &Video_id);

        /**
         *  Sets the format of the VideoData object
         *
         *  @param extension  A string containing the file system
         *    extension corresponding to the desired VideoFormat
         *  @see Video.h for more details on VideoFormat
         */
        void set_format(const std::string &extension);

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
         *  @see Video.h for details on CompressionType
         */


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
        void set_data_from_encoded(const std::vector<unsigned char> &buffer);

        void set_minimum(int dimension);

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
        void read(const std::string &video_id);

        /**
         *  Stores a Write Operation in the list of operations
         *    to perform
         *
         *  @param Video_id  The full path to where the Video should
         *    be written
         *  @param Video_format  The VideoFormat to write the Video in
         *  @param store_metadata  A flag to indicate whether to store the
         *    metadata in TileDB or not. Defaults to true
         *  @see Video.h for more details on VideoFormat
         */
        void write(const std::string &video_id,  bool store_metadata=true);

        // void remove(const std::string &Video_id);

        /**
         *  Stores a Resize Operation in the list of operations
         *    to perform
         *
         *  @param rows  The number of rows in the resized Video
         *  @param columns  The number of columns in the resized Video
         */
        void resize(int rows, int columns);

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
        void copy_cv(const cv::Mat &cv_video);

        /**
         *  Copies the VideoData OpenCV Mat into a buffer
         *
         *  @param buffer  The buffer that will contain the Video
         *    data
         */
        template <class T> void copy_to_buffer(T* buffer);

    /*  *********************** */
    /*      UTIL FUNCTIONS      */
    /*  *********************** */
        /**
         *  Gets the extension of a filename
         *
         *  @param filename  The path to the file
         *  @return The string containing the extension
         */
        std::string get_extension(const std::string &filename);

        /**
         *  Converts VideoFormat to string
         *
         *  @param Video_format The VideoFormat of the Video object
         *  @return The string containing the extension
         */
        std::string format_to_string(VideoFormat Video_format);

        /**
         *  Creates full path to Video with appropriate extension based
         *    on the VideoFormat
         *
         *  @param filename The path to the Video object
         *  @param format  The VideoFormat of the Video object
         *  @return Full path to the object including extension
         */
        std::string create_fullpath(const std::string &filename,
            VideoFormat format);

        /**
         *  Checks to see if the file name is unique by attempting
         *    to open the file
         *
         *  @param name  Full path to the theoretically unique ID
         *  @return True if the file does not exist, false if it does
         */
        bool exists(const std::string &name);
    };

}
