#ifndef VIDEOSUDOKU_IMGPROC_SUDOKU
#define VIDEOSUDOKU_IMGPROC_SUDOKU

#include "include/videosudoku/imgproc/helper.hpp"

namespace videosudoku::imgproc
{
std::optional<contour_t> find_sudoku(cv::Mat const &frame, double const area);
}
#endif
