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

#include <regex>
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
std::string SERVER_IP = "192.168.0.199";
/* ------------- */

/* This structure defines all the possible options for this program.
 * If an option is specified, its argument is also required. */
static struct option long_options[] = {
  {
    "CCTV_CAM",
    required_argument,
    0,
    'c'
  },
  {
    "FPS",
    required_argument,
    0,
    'f'
  },
  {
    "IMG_HEIGHT",
    required_argument,
    0,
    'h'
  },
  {
    "IMG_WIDTH",
    required_argument,
    0,
    'w'
  },
  {
    "SERVER_IP",
    required_argument,
    0,
    's'
  }
};

/* This function converts the string argument of an option to a non-negative integer.
 * If the conversion fails, it throws an exception. */
void read_arg(int & global_target, std::string opt, std::string optarg) {
  try {
    global_target = std::stoi(optarg);
    if (global_target < 0)
      throw std::invalid_argument("Negative integer");
  } catch (std::invalid_argument) {
    std::cout << "Error: Invalid argument value for the " << opt << " option. Expecting non-negative integer.\n";
    exit(EXIT_FAILURE);
  } catch (std::out_of_range) {
    std::cout << "Error: Out of range value for the " << opt << " option.\n";
    exit(EXIT_FAILURE);
  }
}

/* This function checks if the IPv4 address set by the user is in a correct format. */
void read_ipv4(std::string & global_target, std::string optarg) {
  std::__cxx11::regex ipv4("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
  
  if (std::regex_match(optarg, ipv4)) {
    global_target = optarg;
    return;
  }

  std::cout << "Error: The specified IP Address is not a valid IPv4 address.\n";
  exit(EXIT_FAILURE);
}

/* Checks and reads all the command line arguments (if any). */
void read_args(int argc, char * argv[]) {
  int option, option_index;

  /* Loops through all the options (if they exist).
   * The available options are: 
   *
   *  - CCTV_CAM: the webcam index; 
   *  - FPS: frames-per-second for the video capturing;
   *  - IMG_HEIGHT: the height of each frame; 
   *  - IMG_WIDTH: the width of each frame;
   *  - SERVER_IP: The IPv4 address of the server.
   */
  while ((option = getopt_long(argc, argv, "CCTV_CAM::FPS::IMG_HEIGHT::IMG_WIDTH::SERVER_IP::", long_options, & option_index)) != -1) {
    switch (option) {
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
    case 's':
      if (optarg)
	read_ipv4(SERVER_IP, optarg);
      break;
    default:
      abort();

    }
  }
}

void configure_sockaddr_in(sockaddr_in & addr) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(DEFAULT_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

int main(int argc, char * argv[]) {
  /* Read all the command line arguments. */
  read_args(argc, argv);

  int inet_pton_out;
  int sock;
  int sock_fd;
  int bbytee;
  int img_size;
  struct sockaddr_in addr;

  try {

    /* Creating the client-server communication socket. */
    sock = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (sock < 0) {
      throw std::system_error(EFAULT, std::generic_category());
    }

    configure_sockaddr_in(addr);

    /* Converting the IPv4 and IPv6 addresses from text to binary format. */
    inet_pton_out = inet_pton(AF_INET, SERVER_IP.c_str(), & addr.sin_addr);
    
    if (inet_pton_out < 0) {
       throw std::system_error(EFAULT, std::generic_category());
    }

    /* Setting up connection. */
    sock_fd = connect(sock, (struct sockaddr * ) & addr, sizeof(addr));

    if (sock_fd < 0) {
      throw std::system_error(EFAULT, std::generic_category());
    }
    
    /* Capturing the frames from the camera. */
    cv::VideoCapture video_capture(CCTV_CAM);
    video_capture.set(cv::CAP_PROP_FPS, FPS);

    cv::Mat frame;
    frame = cv::Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);

    /* The size of a frame is defined by its width, height and the number of its color channels. */
    img_size = frame.cols * frame.rows * 3;

    while (true) {
      /* Reading the current frame. */
      video_capture.read(frame);

      if (frame.empty())
        throw cv::Exception();
	
      cv::Mat gray_frame;
      
      //cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);

      /* Sending the captured frame to the server. */
      bbytee = send(sock, frame.data, img_size, 0);
      
      //cv::imshow("CCTV_CAM_CLIENT", gray_frame);
      //cv::waitKey(1);
    }

    close(sock);
  } catch (cv::Exception & e) {
    const char * err_msg = e.what();
    std::cout << "Caught exception: " << err_msg << std::endl;
    return -1;
  } catch (std::system_error & e) {
    std::cout << "Caught error: " << e.code() << " - " << e.what() << '\n';
    return -1;
  }

  return 0;
}
