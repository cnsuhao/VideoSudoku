#ifndef VIDEOSUDOKU_SOLVER
#define VIDEOSUDOKU_SOLVER

#include <array>

namespace videosudoku
{
constexpr auto sudoku_nums { 9 };

using sudoku_t = std::array<std::array<uint8_t, sudoku_nums>, sudoku_nums>;

bool complete(sudoku_t &sudoku);
}
#endif
