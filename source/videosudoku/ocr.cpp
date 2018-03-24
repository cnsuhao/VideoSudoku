#include "include/videosudoku/ocr.hpp"

#include <optional>

#include <opencv2/imgproc.hpp>

namespace
{
using namespace videosudoku;

using contour_t = std::vector<cv::Point>;

constexpr auto number_margin { 0.05 };

constexpr auto criteria_cols { 0.10 };

constexpr auto criteria_rows { 0.40 };

constexpr auto default_ratio { 0.90 };

constexpr auto image_size { 30 };

constexpr auto nodes_size { image_size * image_size + 1 };

bool is_centered_number(contour_t const &contour, uint32_t const cols, uint32_t const rows)
{
    auto const rect { cv::boundingRect(contour) };

    return cols * number_margin <= rect.x
        && rows * number_margin <= rect.y
        && cols * criteria_cols <= rect.width
        && rows * criteria_rows <= rect.height;
}

auto select_number(std::vector<contour_t> &contours, uint32_t const cols, uint32_t const rows)
{
    std::sort(contours.begin(), contours.end(), [](auto &lhs, auto &rhs) {
        return cv::contourArea(lhs) > cv::contourArea(rhs);
    });

    return std::find_if(contours.begin(), contours.end(), [&](auto &contour) {
        return is_centered_number(contour, cols, rows);
    });
}

std::optional<cv::Rect> find_number(cv::Mat const &image)
{
    assert(is_binary(image));

    std::vector<contour_t> contours;

    cv::findContours(image.clone(), contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    auto const number { select_number(contours, image.cols, image.rows) };

    return number == contours.end()
        ? std::nullopt
        : std::optional { cv::boundingRect(*number) };
}

void crop_number(cv::Mat &image, cv::Rect const &rect) noexcept
{
    assert(is_binary(image));

    auto const x { std::max(rect.x + rect.width / 2 - rect.height / 2, 0) };

    auto const width { std::min(rect.height, image.cols - rect.x) };

    image = image(cv::Rect { x, rect.y, width, rect.height });
}

void crop_number(cv::Mat &image) noexcept
{
    assert(is_binary(image));

    cv::Rect const rect {
        int32_t(image.cols * number_margin),
        int32_t(image.rows * number_margin),
        int32_t(image.cols * default_ratio),
        int32_t(image.rows * default_ratio)
    };

    image = image(rect);
}

void normalize(cv::Mat &image)
{
    assert(is_binary(image));

    if (auto const rect { find_number(image) })
    {
        crop_number(image, *rect);
    }
    else
    {
        crop_number(image);
    }

    cv::resize(image, image, cv::Size(image_size, image_size));
}

void pack_nodes(std::array<svm_node, nodes_size> &nodes, cv::Mat const &image) noexcept
{
    assert(is_binary(image));
    assert(image.size() == cv::Size(image_size, image_size));

    auto it { image.begin<uint8_t>() };

    for (auto i { 0 }; i < nodes_size - 1; ++i)
    {
        nodes[i] = { i + 1, double(*it++) };
    }
}

std::array<svm_node, nodes_size> to_nodes(cv::Mat const &image) noexcept
{
    assert(is_binary(image));
    assert(image.size() == cv::Size(image_size, image_size));

    std::array<svm_node, nodes_size> nodes;

    pack_nodes(nodes, image);

    nodes.back().index = -1;

    return nodes;
}
}

namespace videosudoku
{
ocr_t::ocr_t(std::string const &path):
    model_ { svm_load_model(path.c_str()) }
{
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
