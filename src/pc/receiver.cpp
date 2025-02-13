#include "../../include/pc/receiver.hpp"

VideoReceiver::VideoReceiver(io_service& ios, const std::string& address,
                                     int port, const std::string& window) 
: ioService(ios), socket(ios), windowName(window), isRunning(true) {
    try {
        tcp::resolver resolver(ioService);
        tcp::resolver::query query(address, std::to_string(port));
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::asio::connect(socket, endpoint_iterator);
        std::cout << "Connected to sender on port " << port << std::endl;
    }
    catch (std::exception& e) {
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
}


void VideoReceiver::receiveFrames() {
    try {
        while (isRunning) {
            // Receive frame size
            uint32_t frameSize;
            boost::asio::read(socket, boost::asio::buffer(&frameSize, sizeof(frameSize)));

            // Receive frame data
            std::vector<uchar> buffer(frameSize);
            boost::asio::read(socket, boost::asio::buffer(buffer));

            // Decode frame
            cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
            if (!frame.empty()) {
                std::lock_guard<std::mutex> lock(frameMutex);
                currentFrame = frame.clone();
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in receiver " << windowName << ": " << e.what() << std::endl;
    }
}


cv::Mat VideoReceiver::getCurrentFrame() {
    std::lock_guard<std::mutex> lock(frameMutex);
    return currentFrame.clone();
}