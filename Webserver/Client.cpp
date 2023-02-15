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
#include <system_error>
#include <errno.h>
/* ----------------- */

/* --- MACROs --- */
#define DEFAULT_PROTOCOL 0
#define DEFAULT_PORT 3425
/* -------------- */

/* --- Globals --- */
int CCTV_CAM = 0;
int FPS = 32;
int IMG_HEIGHT = 480;
int IMG_WIDTH = 640;
/* ------------- */


/* This structure defines all the possible options for this program.
 * If an option is specified, its argument is also required. */
static struct option long_options[] = {
	{"CCTV_CAM", 	required_argument, 	0, 	'c'},
	{"FPS",		    required_argument,	0,	'f'},
    {"IMG_HEIGHT",  required_argument,  0,  'h'},
    {"IMG_WIDTH",   required_argument,  0,  'w'}
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

/* Checks and reads all the command line arguments (if any). */
void read_args(int argc, char *argv[]) {
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
            case 'h':
                if (optarg)
                    read_arg(IMG_HEIGHT, "IMG_HEIGHT", optarg);
                break;
            case 'w':
                if (optarg)
                    read_arg(IMG_WIDTH, "IMG_WIDTH", optarg);
                break;
			default:
				abort();
			
		}
	}
}

void configure_sockaddr_in(sockaddr_in& addr) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

int main(int argc, char *argv[]) {	
    /* Read all the command line arguments. */
    read_args(argc, argv);	

    int sock;
    int connect_out;
    int bbytee;
    struct sockaddr_in addr;

	try {

        /* Creating the client-server communication socket. */
        sock = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
        if (sock < 0) {
            throw std::system_error(EFAULT, std::generic_category());
        }
    
        configure_sockaddr_in(addr);

        /* Setting up connection. */
        connect_out = connect(sock, (struct sockaddr*) &addr, sizeof(addr));

        if (connect_out < 0) {
            throw std::system_error(EFAULT, std::generic_category());
        }
        
		cv::VideoCapture videoCapture(CCTV_CAM);
		videoCapture.set(cv::CAP_PROP_FPS, FPS);
      
 		cv::Mat frame;
        frame = cv::Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);

        int imgSize = frame.cols * frame.rows * 3;

		while (true) {
			videoCapture.read(frame);
            
            if (frame.empty())
                throw cv::Exception();

            bbytee = send(sock, frame.data, imgSize, 0);
    
            cv::imshow("CCTV_CAM", frame);
			cv::waitKey(1);
		}

        close(sock);
        return 0;
	}
	catch (cv::Exception &e) {
		const char *err_msg = e.what();
        std::cout << "Caught exception: " << err_msg << std::endl;
		return -1;
	}
    catch (std::system_error& e) {
        std::cout << "Caught error: " << e.code() << " - " << e.what() << '\n';
        return -1;
    }

	
	return 0;
}
