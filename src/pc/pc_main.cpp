#include "../../include/pc/input_source.hpp"
#include "../../include/pc/overlay_generator.hpp"
#include "../../include/pc/receiver.hpp"


int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <sender1_address> <port1> <sender2_address> <port2>" << std::endl;
        return 1;
    }

    try {
        io_service ioService;

        // Create two video receivers
        VideoReceiver receiver1(ioService, argv[1], std::stoi(argv[2]), "Video Stream 1");
        VideoReceiver receiver2(ioService, argv[3], std::stoi(argv[4]), "Video Stream 2");

        // Create threads for receiving frames
        std::thread receiveThread1(&VideoReceiver::receiveFrames, &receiver1);
        std::thread receiveThread2(&VideoReceiver::receiveFrames, &receiver2);

        // Create windows for all displays
        cv::namedWindow("Thermal Stream", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Regular Stream", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Combined View", cv::WINDOW_AUTOSIZE);

        int threshold = 127;
        double alpha = 0.6;
        Overlay_Generator overlay_generator;

        while (true) {
            // Get current frames from both receivers
            cv::Mat frame1 = receiver1.getCurrentFrame();  // Thermal
            cv::Mat frame2 = receiver2.getCurrentFrame();  // Regular

            if (!frame1.empty() && !frame2.empty()) {
                // Resize frames to ensure they have the same size
                int targetHeight = 480;
                double aspect1 = frame1.cols / (double)frame1.rows;
                double aspect2 = frame2.cols / (double)frame2.rows;
                
                cv::Mat resized1, resized2;
                cv::resize(frame1, resized1, cv::Size(int(targetHeight * aspect1), targetHeight));
                cv::resize(frame2, resized2, cv::Size(int(targetHeight * aspect2), targetHeight));

                // Process overlay (your overlay_thermal function call here)
                cv::Mat overlaid = overlay_generator.overlay_thermal(resized1, resized2, threshold, alpha);

                // Create a combined frame to show all three views
                int totalWidth = resized1.cols + resized2.cols + overlaid.cols;
                cv::Mat combinedFrame(targetHeight, totalWidth, CV_8UC3);
                
                // Copy all three frames into combined view
                resized1.copyTo(combinedFrame(cv::Rect(0, 0, resized1.cols, targetHeight)));
                resized2.copyTo(combinedFrame(cv::Rect(resized1.cols, 0, resized2.cols, targetHeight)));
                overlaid.copyTo(combinedFrame(cv::Rect(resized1.cols + resized2.cols, 0, overlaid.cols, targetHeight)));

                // Display all views
                cv::imshow("Thermal Stream", resized1);
                cv::imshow("Regular Stream", resized2);
                cv::imshow("Combined View", overlaid);
                cv::imshow("All Views", combinedFrame);
            }

            // Handle keyboard input
            char key = (char)cv::waitKey(1);
            if (key == 27) { // ESC key to exit
                break;
            }
            else if (key == 'w') { // Increase threshold
                threshold = std::min(threshold + 5, 255);
                std::cout << "Threshold: " << threshold << std::endl;
            }
            else if (key == 's') { // Decrease threshold
                threshold = std::max(threshold - 5, 0);
                std::cout << "Threshold: " << threshold << std::endl;
            }
            else if (key == 'a') { // Decrease alpha
                alpha = std::max(alpha - 0.1, 0.0);
                std::cout << "Alpha: " << alpha << std::endl;
            }
            else if (key == 'd') { // Increase alpha
                alpha = std::min(alpha + 0.1, 1.0);
                std::cout << "Alpha: " << alpha << std::endl;
            }
        }

        // Cleanup
        receiver1.stop();
        receiver2.stop();
        receiveThread1.join();
        receiveThread2.join();
        cv::destroyAllWindows();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}