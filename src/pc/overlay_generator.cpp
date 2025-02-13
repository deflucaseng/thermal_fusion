#include "../../include/pc/overlay_generator.hpp"

cv::Mat Overlay_Generator::overlay_thermal(const cv::Mat& thermal_frame, 
    const cv::Mat& regular_frame,
    int threshold = -1,
    double alpha = -1) {
    // Get optimal parameters if not provided
    if (threshold == -1 || alpha == -1) {
        DynamicParams params = get_optimal_params(thermal_frame, false);
        if (threshold == -1) threshold = params.threshold;
        if (alpha == -1) alpha = params.alpha;
    }

    // Ensure frames are the same size
    cv::Mat thermal_resized;
    cv::resize(thermal_frame, thermal_resized, regular_frame.size());

    // Process thermal frame to get mask of hot areas
    std::vector<cv::Mat> channels;
    cv::split(thermal_resized, channels);

    // Create mask from red channel
    cv::Mat mask;
    cv::threshold(channels[2], mask, threshold, 255, cv::THRESH_BINARY);

    // Create output frame starting with regular video
    cv::Mat output = regular_frame.clone();

    // Overlay thermal data where mask is active
    cv::Mat thermal_roi;
    thermal_resized.copyTo(thermal_roi, mask);

    // Blend the thermal overlay with the regular frame
    cv::addWeighted(output, 1.0 - alpha, thermal_roi, alpha, 0.0, output, -1);

    return output;
}