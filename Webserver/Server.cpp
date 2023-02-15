/** 
 * --- C++ Webserver ---
 * The server-side of the client-server system paradigm, created for receiving video frames from the clients.
 * This program uses multi-threading in order to handle multiple clients (camera inputs) in a concurrent manner.
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
#include <pthread.h>
/* ----------------- */

/* --- MACROs --- */
#define DEFAULT_PROTOCOL 0
#define DEFAULT_PORT 3425
#define DEFAULT_BACKLOG 10
/* -------------- */

/* --- Globals --- */
int BACKLOG = 10;
int IMG_WIDTH = 640;
int IMG_HEIGHT = 480;
/* --------------- */


/* This structure defines all the possible options for this program.
 * If an option is specified, its argument is also required. */
static struct option long_options[] = {
    {"IMG_HEIGHT",  required_argument,  0,  'h'},
    {"IMG_WIDTH",   required_argument,  0,  'w'}
};


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
void read_args(int argc, char*argv[]) {
    int option, option_index;

    while ((option = getopt_long(argc, argv, "IMG_HEIGHT::IMG_WIDTH::", long_options, &option_index)) != -1) {
        switch(option) {
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

void configure_sockaddr_in(sockaddr_in &addr) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

void *handle_client(void *socket) {
    int num_of_recv_bytes;
    int img_size;
    int key;
    int sock;
    cv::Mat frame;
    uchar *frameptr;
    cv::VideoWriter outputVideo;
    cv::Size S;

    sock = (int)socket;
    img_size = IMG_WIDTH * IMG_HEIGHT * 3; 
    frame = cv::Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
    frameptr = frame.data;
    S = cv::Size(IMG_WIDTH, IMG_HEIGHT);
    
    outputVideo.open("receive.mp4", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, S, true);

    while (true) {
        num_of_recv_bytes = recv(sock, frameptr, img_size, MSG_WAITALL);
        cv::imshow("CCTV_CAM_SERVER", frame);
        key = cv::waitKey(100);

       if (key >= 0) break;
    }

    outputVideo.release();
    close(sock);

    return NULL;
}

int main() {
    int sock;
    int listener;
    int bind_out;
    struct sockaddr_in addr;

    try {
        listener = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
        if (listener < 0) {
            throw std::system_error(EFAULT, std::generic_category());
        }

        configure_sockaddr_in(addr);

        /* Bind the socket to the corresponding address and port,
         * specified inside the sockaddr struct object. */
        bind_out = bind(listener, (struct sockaddr*) &addr, sizeof(addr));
        if (bind_out < 0) {
            throw std::system_error(EFAULT, std::generic_category());
        }

        /* Start listening and await future client connections. */
        listen(listener, BACKLOG);       

        while (true) {
            sock = accept(listener, NULL, NULL);

            if (sock >= 0) {
                pthread_t thread;
                pthread_create(&thread, NULL, handle_client, (void*) sock);
            }           
        } 
    }
    catch (std::system_error &e) {
        std::cout << "Caught error: " << e.code() << " - " << e.what() << '\n';
        return -1;
    }

    return 0;
}
