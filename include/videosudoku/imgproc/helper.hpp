#ifndef VIDEOSUDOKU_IMGPROC_HELPER
#define VIDEOSUDOKU_IMGPROC_HELPER

#include <opencv2/imgproc.hpp>

namespace videosudoku::imgproc
{
using contour_t = std::vector<cv::Point>;

constexpr auto thresh_block { 23 };

constexpr auto thresh_const { 5.5 };

inline void to_binary(cv::Mat &frame, int32_t const type)
{
    assert(frame.type() == CV_8UC3);

    cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

    cv::adaptiveThreshold(frame, frame, 255, cv::ADAPTIVE_THRESH_MEAN_C, type, thresh_block, thresh_const);
}

inline bool greater_by_area(contour_t const &lhs, contour_t const &rhs)
{
    return cv::contourArea(lhs) > cv::contourArea(rhs);
}

inline bool greater_than(contour_t const &contour, double const area)
{
    return cv::contourArea(contour) > area;
}
}
#endif
