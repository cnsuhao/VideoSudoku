#ifndef VIDEOSUDOKU_OCR
#define VIDEOSUDOKU_OCR

#include <opencv2/core.hpp>
#include <svm.h>

namespace videosudoku
{
class ocr_t final
{
    svm_model *model_ { nullptr };

public:
    explicit ocr_t(std::string const &path);

    ~ocr_t();

    ocr_t(ocr_t &&) = delete;

    uint8_t recognize(cv::Mat const &image) const;
};

class ocr_exception : public std::exception {};

constexpr bool is_binary(cv::Mat const &image) noexcept
{
    return image.dims == 2 && image.type() == CV_8UC1;
}
}
#endif
