#include "include/videosudoku/ocr.hpp"

#include <opencv2/imgproc.hpp>

namespace
{
using namespace videosudoku;

using contour_t = std::vector<cv::Point>;

using contours_t = std::vector<contour_t>;

constexpr auto number_margin { 0.05 };

constexpr auto criteria_cols { 0.1 };

constexpr auto criteria_rows { 0.4 };

constexpr auto default_ratio { 0.9 };

constexpr auto image_rc { 30 };

constexpr auto data_len { image_rc * image_rc + 1 };

bool is_centered_number(contour_t const &contour, double const cols, double const rows)
{
    auto const rect { cv::boundingRect(contour) };

    return cols * number_margin <= rect.x
        && rows * number_margin <= rect.y
        && cols * criteria_cols <= rect.width
        && rows * criteria_rows <= rect.height;
}

contour_t const *select_number(contours_t const &contours, double const cols, double const rows)
{
    contour_t const *selected { nullptr };

    auto max_area { 0.0 };

    for (auto const &contour : contours)
    {
        if (!is_centered_number(contour, cols, rows)) continue;

        if (auto const area { cv::contourArea(contour) }; area > max_area)
        {
            selected = &contour;

            max_area = area;
        }
    }

    return selected;
}

void crop_number(cv::Mat &image, contour_t const &contour)
{
    assert(is_binary(image));

    auto const rect { cv::boundingRect(contour) };

    auto const x { std::max(rect.x + rect.width / 2 - rect.height / 2, 0) };

    auto const width { x + rect.height > image.cols ? image.cols - x : rect.height };

    image = image(cv::Rect(x, rect.y, width, rect.height));
}

void crop_number(cv::Mat &image)
{
    assert(is_binary(image));

    cv::Rect const rect(
        image.cols * number_margin,
        image.rows * number_margin,
        image.cols * default_ratio,
        image.rows * default_ratio);

    image = image(rect);
}

void normalize(cv::Mat &image)
{
    assert(is_binary(image));

    contours_t contours;

    cv::findContours(image.clone(), contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    if (auto const contour { select_number(contours, image.cols, image.rows) })
    {
        crop_number(image, *contour);
    }
    else
    {
        crop_number(image);
    }

    cv::resize(image, image, cv::Size(image_rc, image_rc));
}

std::array<svm_node, data_len> to_nodes(cv::Mat const &image)
{
    assert(is_binary(image));
    assert(image.cols == image_rc);
    assert(image.rows == image_rc);

    std::array<svm_node, data_len> nodes;

    auto it { image.begin<uint8_t>() };

    for (auto i { 0 }; i < data_len - 1; ++i)
    {
        nodes[i] = { i + 1, double(*it++) };
    }

    nodes.back().index = -1;

    return nodes;
}
}

namespace videosudoku
{
ocr_t::ocr_t(std::string const &path)
{
    model_ = svm_load_model(path.c_str());

    if (!model_) throw ocr_exception {};
}

ocr_t::~ocr_t()
{
    svm_free_and_destroy_model(&model_);
}

uint8_t ocr_t::recognize(cv::Mat const &image) const
{
    assert(is_binary(image));

    auto tmp { image.clone() };

    normalize(tmp);

    return svm_predict(model_, to_nodes(tmp).data());
}
}
