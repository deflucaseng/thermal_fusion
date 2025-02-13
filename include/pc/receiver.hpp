#ifndef RECEIVER_HPP
#define RECEIVER_HPP


#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

using namespace boost::asio;
using ip::tcp;

class VideoReceiver {
    private:
        io_service& ioService;
        tcp::socket socket;
        std::string windowName;
        cv::Mat currentFrame;
        std::mutex frameMutex;
        bool isRunning;

    public:
        VideoReceiver(io_service& ios, const std::string& address, int port, const std::string& window);

        void receiveFrames();

        cv::Mat getCurrentFrame();

        void stop() {isRunning = false;}
};

#endif