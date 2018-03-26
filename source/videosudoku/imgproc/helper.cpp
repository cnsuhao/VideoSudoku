#include "include/videosudoku/imgproc/helper.hpp"

#include <range/v3/action/sort.hpp>

namespace
{
using namespace videosudoku::imgproc;

constexpr auto thresh_block { 23 };

constexpr auto thresh_const { 5.5 };
}

namespace videosudoku::imgproc
{
void to_binary(cv::Mat const &src, cv::Mat &dst, int32_t const type)
{
    assert(src.type() == CV_8UC3);

    cv::cvtColor(src, dst, cv::COLOR_RGB2GRAY);

    cv::adaptiveThreshold(dst, dst, 255, cv::ADAPTIVE_THRESH_MEAN_C, type, thresh_block, thresh_const);
}

std::vector<contour_t> find_contours(cv::Mat const &image, int32_t const mode)
{
    assert(image.type() == CV_8UC1);

    std::vector<contour_t> contours;

    cv::findContours(image.clone(), contours, mode, cv::CHAIN_APPROX_SIMPLE);

    return contours |= ranges::action::sort(greater_by_area);
}
}
