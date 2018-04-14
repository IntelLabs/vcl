#include <opencv2/highgui.hpp>
#include <stddef.h>
#include <iostream>

#include "Video.h"


using namespace VCL;

 using namespace cv; // OpenCV namespace

void VCL::Video::read_video() {

  if(!_inputVideo.isOpened())
  {
    throw "Error opening video stream" ;
  //  return -1;
  }
  std::cout << "Opening video stream" << std::endl;
  // Default resolution of the frame is obtained.The default resolution is system dependent.
   frame_width = _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
   frame_height = _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);

   // string::size_type pAt = source.find_last_of('.');                  // Find extension point
  int ex = static_cast<int>(_inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // Transform from int to char via Bitwise operators
   char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

   Size S = Size((int) _inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                 (int) _inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
    int length = (int) _inputVideo.get(CV_CAP_PROP_FRAME_COUNT);

  std::cout <<" The Video Length is " << length << "\t" << S << std::endl;

}


std::string VCL::Video::get_video (void) {

    _outputVideo= cv::VideoWriter("outcpp.avi",CV_FOURCC('M','J','P','G'),130, Size(frame_width,frame_height));

  while(1)
  {
    Mat frame;

    // Capture frame-by-frame
    _inputVideo >> frame;

    // If the frame is empty, break immediately
    if (frame.empty())
      break;

    // Write the frame into the file 'outcpp.avi'
    _outputVideo.write(frame);

    // Display the resulting frame
    imshow( "Frame", frame );

    // Press  ESC on keyboard to  exit
    char c = (char)waitKey(1);
    if( c == 27 )
      break;
  }

  // When everything done, release the video capture and write object


  return "outcpp.avi";
}

Video::Video(const std::string &fileName)
{
  path 				= fileName;
  std::cout<< " File name is " << path << std::endl;

  // std::ofstream f (fileName, std::ofstream::out);
  //   f.close();

  // path ="/home/ragaad/newVCL/vcl/test/videos/pedestrian1.avi";
    _inputVideo = cv::VideoCapture(fileName);


  // Check if camera opened successfully


     // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file.

}

Video::Video(const cv::VideoCapture &cv_video )
{

}
Video:: Video (void* buffer, int size )
{

}

 void Video::operator=(const Video &vid){

    //  VideoData *temp = _video;
    // _video = new VideoData(*vid._video);
    // delete temp;

 }



Video::~Video()
{
  _inputVideo.release();
  _outputVideo.release();

  // Closes all the windows
  destroyAllWindows();
  delete _video;
}

std::string Video::get_video_id() const
{
  return "id";

}


int Video::processVideo( const std::string &video_id)
{
	//total_begin = clock();

// 	cv::Mat prevFrame;
// 	cv::Mat prevFrameGray;
// 	cv::Mat currFrame;
// 	cv::Mat currFrameGray;



// ////////////-- Open the video and extract first frame
// 	cv::VideoCapture capture(video_id.c_str());

// 	if(!capture.isOpened())
// 	{
// 		printf("cannot open file %s. Exiting...\n", video_id.c_str());
// 		return 0;
// 	}

// ////////////-- Get first frame and video properties
// 	capture >> prevFrame;
// 	if (prevFrame.empty()){
// 		printf("Error reading frame \n");
// 		exit(0);
// 	}

// 	fps 			= capture.get(CV_CAP_PROP_FPS);
// 	frame_count 	= capture.get(CV_CAP_PROP_FRAME_COUNT);
// 	double  codec	= capture.get(CV_CAP_PROP_FOURCC);
// 	// height			= FRAME_HEIGHT;
// 	// width			= FRAME_WIDTH;

// 	printf("processing video %s ... ", this->file_name.c_str());
// 	printf("FPS: %f FRAMES: %f duration: %f codec: %f\n", fps, frame_count, frame_count / fps, codec);

// 	//cv::resize(prevFrame, prevFrame, cv::Size(FRAME_WIDTH, FRAME_HEIGHT));
// 	cvtColor(prevFrame, prevFrameGray, CV_RGB2GRAY);


// ////////////-- Write video name in correct files file
// 	std::ofstream output_file;
// 	// output_file.open(Globals::train_correct_files_fn.c_str(), std::ios::out | std::ios::app);
// 	// output_file << video_id + "\n";
// 	output_file.close();

	return 0;
}