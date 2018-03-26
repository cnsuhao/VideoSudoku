#ifndef VIDEOSUDOKU_IMGPROC_HELPER
#define VIDEOSUDOKU_IMGPROC_HELPER

#include <opencv2/imgproc.hpp>

namespace videosudoku::imgproc
{
using contour_t = std::vector<cv::Point>;

template<typename T, typename F>
constexpr T init_with(F const &func_with_ref)
{
    T value;

    func_with_ref(value);

    return value;
}

inline bool greater_by_area(contour_t const &lhs, contour_t const &rhs)
{
    return cv::contourArea(lhs) > cv::contourArea(rhs);
}

inline bool greater_than(contour_t const &contour, double const area)
{
    return cv::contourArea(contour) > area;
}

void to_binary(cv::Mat const &src, cv::Mat &dst, int32_t type);

std::vector<contour_t> find_contours(cv::Mat const &image, int32_t mode);
}
#endif
