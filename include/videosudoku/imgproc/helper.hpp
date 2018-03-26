#ifndef VIDEOSUDOKU_IMGPROC_HELPER
#define VIDEOSUDOKU_IMGPROC_HELPER

#include <opencv2/imgproc.hpp>

namespace videosudoku::imgproc
{
using contour_t = std::vector<cv::Point>;

void to_binary(cv::Mat &frame, int32_t const type);

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
