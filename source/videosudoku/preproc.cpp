#include "include/videosudoku/preproc.hpp"

#include <opencv2/imgproc.hpp>
#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace
{
using namespace videosudoku;

using contour_t = std::vector<cv::Point>;

constexpr auto approx_ratio { 0.01 };

bool is_sudoku(contour_t const &contour)
{
    return contour.size() == 4 && cv::isContourConvex(contour);
}

cv::Mat to_poly(contour_t const &contour)
{
    cv::Mat approx;

    auto const epsilon { approx_ratio * cv::arcLength(contour, true) };

    cv::approxPolyDP(contour, approx, epsilon, true);

    return approx;
}

std::optional<contour_t> select_sudoku(std::vector<contour_t> const &contours, double const area)
{
    auto candidates {
        contours
            | ranges::view::filter([&](auto &contour) {
                return cv::contourArea(contour) > area;
            })
            | ranges::view::transform(to_poly)
    };

    auto const sudoku { ranges::find_if(candidates, is_sudoku) };

    return sudoku == candidates.end()
        ? std::nullopt
        : std::optional { *sudoku };
}

std::optional<contour_t> find_sudoku(cv::Mat const &frame, double const area)
{
    assert(frame.dims == 2);
    assert(frame.type() == CV_8UC1);

    std::vector<contour_t> contours;

    cv::findContours(frame.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    contours |= ranges::action::sort([](auto &lhs, auto &rhs) {
        return cv::contourArea(lhs) > cv::contourArea(rhs);
    });

    return select_sudoku(contours, area);
}
}

namespace videosudoku
{
}
