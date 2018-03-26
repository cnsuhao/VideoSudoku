#include "include/videosudoku/imgproc/helper.hpp"

namespace
{
using namespace videosudoku::imgproc;

constexpr auto thresh_block { 23 };

constexpr auto thresh_const { 5.5 };
}

namespace videosudoku::imgproc
{
void to_binary(cv::Mat &frame, int32_t const type)
{
    assert(frame.type() == CV_8UC3);

    cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

    cv::adaptiveThreshold(frame, frame, 255, cv::ADAPTIVE_THRESH_MEAN_C, type, thresh_block, thresh_const);
}
}
