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


/* --- MACROs --- */
#define CCTV_CAM 0
#define FPS 32
/* ------------- */


int main() {

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
