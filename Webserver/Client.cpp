/** 
 * --- C++ Webserver ---
 * The client-side of the client-server system paradigm, created for sending video frames to the central webserver.
 * Code written by Eduard-Pavel Miclos as part of the thesis on Surveillance Systems
 * using Convolutional Neural Networks.
 *
 * Politehnica University of Timisoara,
 * 	Faculty of Automatic Control and Computer Science,
 * 		Informatics, 3rd year
 * ---------------------
*/

/* --- Libraries --- */
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
/* ----------------- */


/* --- Globals --- */
int CCTV_CAM = 0;
int FPS = 32;
/* ------------- */


/* This structure defines all the possible options for this program.
 * If an option is specified, its argument is also required. */
static struct option long_options[] = {
	{"CCTV_CAM", 	required_argument, 	0, 	'c'},
	{"FPS",		required_argument,	0,	'f'}

};


/* This function converts the string argument of an option to a non-negative integer.
 * If the conversion fails, it throws an exception. */
void read_arg(int &global_target, std::string opt, std::string optarg) {
	try {
		global_target = std::stoi(optarg);
		if (global_target < 0)
			throw std::invalid_argument("Negative integer");
	}
	catch (std::invalid_argument) {
		std::cout << "Error: Invalid argument value for the " << opt << " option. Expecting non-negative integer.\n";
		exit(EXIT_FAILURE);
	}
	catch (std::out_of_range) {
		std::cout << "Error: Out of range value for the " << opt << " option.\n";
		exit(EXIT_FAILURE);
	}
}


int main(int argc, char *argv[]) {
	
	int option, option_index;

	/** Loops through all the options (if they exist).
	 *  The available options are CCTV_CAM and FPS, letting the user change the webcam index and the FPS for the video capturing.
	 *  This is useful, because different neural networks expect inputs having different FPS. */
	while ((option = getopt_long(argc, argv, "CCTV_CAM::FPS::", long_options, &option_index)) != -1) {
		switch(option) {
			case 'c':
				if (optarg)
					read_arg(CCTV_CAM, "CCTV_CAM", optarg);				
				break;
			case 'f':
				if (optarg)
					read_arg(FPS, "FPS", optarg);
				break;
			default:
				abort();
			
		}
	}


	try {
		cv::VideoCapture videoCapture(CCTV_CAM);
		videoCapture.set(cv::CAP_PROP_FPS, FPS);

		cv::Mat frame;

		while (true) {
			videoCapture.read(frame);
			cv::imshow("CCTV_CAM", frame);

			cv::waitKey(1);
		}
	}
	catch (cv::Exception &e) {
		const char *err_msg = e.what();
		std::cout << "Caught exception: " << err_msg << std::endl;
		return -1;
	}

	
	return 0;
}