#include "include/videosudoku/imgproc/sudoku.hpp"

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace
{
using namespace videosudoku::imgproc;

using namespace std::placeholders;

constexpr auto approx_ratio { 0.01 };

bool is_sudoku(contour_t const &contour)
{
    return contour.size() == 4 && cv::isContourConvex(contour);
}

contour_t to_poly(contour_t const &contour)
{
    auto const epsilon { approx_ratio * cv::arcLength(contour, true) };

    return init_with<contour_t>(std::bind(cv::approxPolyDP, contour, _1, epsilon, true));
}

std::optional<contour_t> select_sudoku(std::vector<contour_t> const &contours, double const area)
{
    auto candidates {
        contours
            | ranges::view::filter(std::bind(greater_than, _1, area))
            | ranges::view::transform(to_poly)
    };

    auto const sudoku { ranges::find_if(candidates, is_sudoku) };

    return sudoku == candidates.end()
        ? std::nullopt
        : std::optional { *sudoku };
}
}

namespace videosudoku::imgproc
{
std::optional<contour_t> find_sudoku(cv::Mat const &image, double const area)
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC3);

    auto const binary { init_with<cv::Mat>(std::bind(to_binary, image, _1, cv::THRESH_BINARY_INV)) };

    auto const contours { find_contours(binary, cv::RETR_EXTERNAL) };

    return select_sudoku(contours, area);
}
}
