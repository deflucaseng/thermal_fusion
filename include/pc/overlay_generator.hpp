#ifndef OVERLAY_GENERATOR_HPP
#define OVERLAY_GENERATOR_HPP

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Overlay_Generator {
public:
    struct DynamicParams {
        double alpha;
        int threshold;
    };

    cv::Mat overlay_thermal(const cv::Mat& thermal_frame, 
                          const cv::Mat& regular_frame,
                          int threshold,
                          double alpha);

private:
    cv::Mat prev_frame;
    double prev_alpha = 0.6;
    int prev_threshold = 127;

    DynamicParams calculate_dynamic_params(const cv::Mat& thermal_frame) {
        std::vector<cv::Mat> channels;
        cv::split(thermal_frame, channels);
        
        // Calculate histogram
        cv::Mat hist;
        int histSize = 256;
        float range[] = {0, 256};
        const float* histRange = {range};
        cv::calcHist(&channels[2], 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
        
        // Find percentiles
        float total = thermal_frame.rows * thermal_frame.cols;
        float sum = 0;
        int thresh_val = 0;
        
        // Find value at 90th percentile for threshold
        for(int i = 0; i < histSize; i++) {
            sum += hist.at<float>(i);
            if(sum / total > 0.9) {
                thresh_val = i;
                break;
            }
        }
        
        // Calculate standard deviation for alpha
        cv::Scalar mean, stddev;
        cv::meanStdDev(channels[2], mean, stddev);
        
        // Higher standard deviation = more contrast = higher alpha
        double dynamic_alpha = std::min(0.4 + (stddev[0] / 255.0), 0.8);
        
        return {dynamic_alpha, thresh_val};
    }

    DynamicParams calculate_motion_based_params(const cv::Mat& current_frame) {
        if (prev_frame.empty()) {
            prev_frame = current_frame.clone();
            return {prev_alpha, prev_threshold};
        }
        
        // Calculate motion
        cv::Mat flow;
        cv::calcOpticalFlowFarneback(prev_frame, current_frame, flow, 
                                   0.5, 3, 15, 3, 5, 1.2, 0);
        
        // Calculate average motion magnitude
        cv::Mat magnitude, angle;
        cv::cartToPolar(flow.col(0), flow.col(1), magnitude, angle);
        double avg_motion = cv::mean(magnitude)[0];
        
        // Adjust parameters based on motion
        double new_alpha = std::max(0.3, prev_alpha - (avg_motion * 0.1));
        int new_threshold = std::max(80, prev_threshold - static_cast<int>(avg_motion * 5));
        
        prev_frame = current_frame.clone();
        prev_alpha = new_alpha;
        prev_threshold = new_threshold;
        
        return {new_alpha, new_threshold};
    }

    DynamicParams get_optimal_params(const cv::Mat& thermal_frame, bool is_moving) {
        DynamicParams temp_params = calculate_dynamic_params(thermal_frame);
        
        if (is_moving) {
            temp_params.alpha *= 0.8;
            temp_params.threshold = static_cast<int>(temp_params.threshold * 0.9);
        }
        
        temp_params.alpha = std::max(0.3, std::min(temp_params.alpha, 0.8));
        temp_params.threshold = std::max(80, std::min(temp_params.threshold, 200));
        
        return temp_params;
    }
};




















#endif