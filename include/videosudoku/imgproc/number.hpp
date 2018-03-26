#ifndef VIDEOSUDOKU_IMGPROC_NUMBER
#define VIDEOSUDOKU_IMGPROC_NUMBER

#include <opencv2/core.hpp>

namespace videosudoku::imgproc
{
void normalize_number(cv::Mat const &src, cv::Mat &dst);
}
#endif
