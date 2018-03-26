#include "include/videosudoku/ocr.hpp"

#include <opencv2/imgproc.hpp>

namespace
{
using namespace videosudoku;

constexpr auto image_size { 30 };

constexpr auto nodes_size { image_size * image_size + 1 };

void pack_nodes(std::array<svm_node, nodes_size> &nodes, cv::Mat const &image) noexcept
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);
    assert(image.size() == cv::Size(image_size, image_size));

    auto it { image.begin<uint8_t>() };

    for (auto i { 0 }; i < nodes_size - 1; ++i)
    {
        nodes[i] = { i + 1, double(*it++) };
    }
}

std::array<svm_node, nodes_size> to_nodes(cv::Mat const &image) noexcept
{
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);
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
    assert(image.dims == 2);
    assert(image.type() == CV_8UC1);

    cv::resize(image, working, cv::Size { image_size, image_size });

    return svm_predict(model_, to_nodes(working).data());
}
}
