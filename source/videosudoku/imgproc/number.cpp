#include "include/videosudoku/imgproc/number.hpp"

#include <functional>
#include <optional>

#include <range/v3/action/sort.hpp>
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

std::optional<cv::Rect> select_number(std::vector<contour_t> const &contours, int32_t const cols, int32_t const rows)
{
    auto const number {
        ranges::find_if(contours, std::bind(is_centered_number, _1, cols, rows))
    };

    return number == contours.end()
        ? std::nullopt
        : std::optional { cv::boundingRect(*number) };
}

std::optional<cv::Rect> find_number(cv::Mat const &image)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    std::vector<contour_t> contours;

    cv::findContours(image.clone(), contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    contours |= ranges::action::sort(greater_by_area);

    return select_number(contours, image.cols, image.rows);
}

void crop_number(cv::Mat &image, cv::Rect const &rect)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    auto const [x, y, w, h] { rect };

    auto const new_x { std::max(0, x + w / 2 - h / 2) };

    auto const new_w { std::min(h, image.cols - new_x) };

    image = image(cv::Rect { new_x, y, new_w, h });
}

void crop_number(cv::Mat &image)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    cv::Rect const rect {
        int32_t(image.cols * number_margin),
        int32_t(image.rows * number_margin),
        int32_t(image.cols * default_ratio),
        int32_t(image.rows * default_ratio)
    };

    image = image(rect);
}
}

namespace videosudoku::imgproc
{
void normalize_number(cv::Mat &image)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC3);

    to_binary(image, cv::THRESH_BINARY);

    if (auto const rect { find_number(image) })
    {
        return crop_number(image, *rect);
    }

    crop_number(image);
}
}
