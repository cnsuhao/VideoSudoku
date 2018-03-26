#include "include/videosudoku/imgproc/number.hpp"

#include <range/v3/algorithm/find_if.hpp>

#include "include/videosudoku/imgproc/helper.hpp"

namespace
{
using namespace videosudoku::imgproc;

using namespace std::placeholders;

constexpr auto number_margin { 0.05 };

constexpr auto criteria_cols { 0.10 };

constexpr auto criteria_rows { 0.40 };

constexpr auto default_ratio { 0.90 };

bool is_centered_number(contour_t const &contour, int32_t const cols, int32_t const rows)
{
    auto const [x, y, w, h] { cv::boundingRect(contour) };

    return x >= cols * number_margin
        && y >= rows * number_margin
        && w >= cols * criteria_cols
        && h >= rows * criteria_rows;
}

std::optional<cv::Rect> find_number(cv::Mat const &image)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    auto const contours { find_contours(image, cv::RETR_CCOMP) };

    auto const number { ranges::find_if(contours, std::bind(is_centered_number, _1, image.cols, image.rows)) };

    return number == contours.end()
        ? std::nullopt
        : std::optional { cv::boundingRect(*number) };
}

cv::Mat roi_number(cv::Mat const &image, cv::Rect const &rect)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    auto const [x, y, w, h] { rect };

    auto const new_x { std::max(0, x + w / 2 - h / 2) };

    auto const new_w { std::min(h, image.cols - new_x) };

    return image(cv::Rect { new_x, y, new_w, h });
}

cv::Mat roi_number(cv::Mat const &image)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    cv::Rect const rect {
        int32_t(image.cols * number_margin),
        int32_t(image.rows * number_margin),
        int32_t(image.cols * default_ratio),
        int32_t(image.rows * default_ratio)
    };

    return image(rect);
}
}

namespace videosudoku::imgproc
{
void normalize_number(cv::Mat const &src, cv::Mat &dst)
{
    assert(src.dims == 2);
    assert(src.type() == CV_8UC3);

    to_binary(src, dst, cv::THRESH_BINARY);

    auto const rect { find_number(dst) };

    dst = rect ? roi_number(dst, *rect) : roi_number(dst);
}
}
