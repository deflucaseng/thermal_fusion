#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

using namespace boost::asio;
using ip::tcp;

class VideoSender {
private:
    cv::VideoCapture videoCapture;
    io_service& ioService;
    tcp::acceptor acceptor;
    std::chrono::steady_clock::time_point lastFrameTime;
    double fps;
    std::string videoPath;

public:
    VideoSender(io_service& ios, const std::string& vPath, int port) 
        : ioService(ios), acceptor(ios, tcp::endpoint(tcp::v4(), port)), videoPath(vPath) {
        if (!videoCapture.open(videoPath)) {
            throw std::runtime_error("Failed to open video file: " + videoPath);
        }
        fps = videoCapture.get(cv::CAP_PROP_FPS);
        lastFrameTime = std::chrono::steady_clock::now();
    }

    void start() {
        try {
            std::cout << "Waiting for connection on port " << acceptor.local_endpoint().port() << std::endl;
            tcp::socket socket(ioService);
            acceptor.accept(socket);
            std::cout << "Client connected for video: " << videoPath << std::endl;
            streamVideo(socket);
        }
        catch (std::exception& e) {
            std::cerr << "Exception in sender " << videoPath << ": " << e.what() << std::endl;
        }
    }

private:
    void streamVideo(tcp::socket& socket) {
        cv::Mat frame;
        std::vector<uchar> buffer;

        while (videoCapture.read(frame)) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - lastFrameTime).count();
            
            int frameDelay = static_cast<int>((1000.0 / fps) - elapsedTime);
            if (frameDelay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay));
            }

            cv::imencode(".jpg", frame, buffer);

            try {
                uint32_t size = buffer.size();
                boost::asio::write(socket, boost::asio::buffer(&size, sizeof(size)));
                boost::asio::write(socket, boost::asio::buffer(buffer));
            }
            catch (std::exception& e) {
                std::cerr << "Connection lost: " << e.what() << std::endl;
                break;
            }

            lastFrameTime = std::chrono::steady_clock::now();
        }

        videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <video1_path> <video2_path> <port1> <port2>" << std::endl;
        return 1;
    }

    try {
        io_service ioService;
        
        std::string videoPath1 = argv[1];
        std::string videoPath2 = argv[2];  // Second video path
        int port1 = std::stoi(argv[3]);
        int port2 = std::stoi(argv[4]);
        
        VideoSender sender1(ioService, videoPath1, port1);
        VideoSender sender2(ioService, videoPath2, port2);

        // Create threads for each sender
        std::thread senderThread1(&VideoSender::start, &sender1);
        std::thread senderThread2(&VideoSender::start, &sender2);

        // Wait for both threads to complete
        senderThread1.join();
        senderThread2.join();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}