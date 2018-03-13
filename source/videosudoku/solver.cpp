#include "include/videosudoku/solver.hpp"

#include "library/dlxcc/include/dlxcc/dlx.hpp"

namespace
{
using namespace videosudoku;

constexpr auto box_side { 3 };

constexpr auto cond_num { 4 };

constexpr auto dlx_rows { sudoku_nums * sudoku_nums * sudoku_nums };

constexpr auto dlx_cols { sudoku_nums * sudoku_nums * cond_num };

constexpr dlxcc::rows_t to_dlx_row(uint8_t const row, uint8_t const col, uint8_t const num) noexcept
{
    assert(row < sudoku_nums);
    assert(col < sudoku_nums);
    assert(num < sudoku_nums);

    return (row * sudoku_nums + col) * sudoku_nums + num;
}

constexpr dlxcc::cols_t to_dlx_col(uint8_t const cnd, uint8_t const lhs, uint8_t const rhs) noexcept
{
    assert(cnd < cond_num);
    assert(lhs < sudoku_nums);
    assert(rhs < sudoku_nums);

    return (cnd * sudoku_nums + lhs) * sudoku_nums + rhs;
}

constexpr uint8_t to_sudoku_box(uint8_t const row, uint8_t const col) noexcept
{
    assert(row < sudoku_nums);
    assert(col < sudoku_nums);

    return row / box_side * box_side + col / box_side;
}

constexpr uint8_t to_sudoku_row(dlxcc::rows_t const row) noexcept
{
    assert(row < dlx_rows);

    return row / sudoku_nums / sudoku_nums;
}

constexpr uint8_t to_sudoku_col(dlxcc::rows_t const row) noexcept
{
    assert(row < dlx_rows);

    return row / sudoku_nums % sudoku_nums;
}

constexpr uint8_t to_sudoku_num(dlxcc::rows_t const row) noexcept
{
    assert(row < dlx_rows);

    return row % sudoku_nums;
}

constexpr void set_all_cell(dlxcc::dlx_t &dlx)
{
    for (uint8_t num { 0 }; num < sudoku_nums; ++num)
    {
        for (uint8_t row { 0 }; row < sudoku_nums; ++row)
        {
            for (uint8_t col { 0 }; col < sudoku_nums; ++col)
            {
                auto const dlx_row { to_dlx_row(row, col, num) };

                dlx.set(dlx_row, to_dlx_col(0, row, col));
                dlx.set(dlx_row, to_dlx_col(1, row, num));
                dlx.set(dlx_row, to_dlx_col(2, col, num));
                dlx.set(dlx_row, to_dlx_col(3, to_sudoku_box(row, col), num));
            }
        }
    }
}

constexpr void set_problem(dlxcc::dlx_t &dlx, sudoku_t const &problem)
{
    for (uint8_t row { 0 }; row < sudoku_nums; ++row)
    {
        for (uint8_t col { 0 }; col < sudoku_nums; ++col)
        {
            if (auto const num { problem[row][col] }; num < sudoku_nums)
            {
                dlx.select(to_dlx_row(row, col, num));
            }
        }
    }
}
}

namespace videosudoku
{
bool complete(sudoku_t &sudoku)
{
    dlxcc::dlx_t dlx { dlx_rows, dlx_cols };

    set_all_cell(dlx);

    set_problem(dlx, sudoku);

    if (!dlx.solve()) return false;

    for (auto const result : dlx.results())
    {
        sudoku[to_sudoku_row(result)][to_sudoku_col(result)] = to_sudoku_num(result);
    }

    return true;
}
}
