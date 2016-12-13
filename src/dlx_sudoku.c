//!
//! @file      dlx_sudoku.c
//! @brief     dlx_sudoku モジュール実装
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "dlx_sudoku.h"
#include "dlx.h"

#define N 9
#define N_ROW N
#define N_COL N
#define N_BOX N
#define N_BOX_SIDE 3
#define N_CELL N_ROW * N_COL
#define N_TYPE_COL 4

#define PROBLEM_FILE1 "test/top95.txt"
#define PROBLEM_FILE2 "test/diff.txt"
#define PROBLEM_FILE3 "test/hardest.txt"

static int solve_dlx_sudoku_cb(int nsolution, int *solutions, void *solved_cb_param);
static void dlx_set_all_cell(dlx_t *dlx);
static void set_dlx_sudoku_problem(dlx_t *dlx, const char *problem);

static int to_dlx_row(int row, int col, int num)
{
    return (((row * N) + col) * N) + num;
}

static int to_dlx_col(int type, int a, int b)
{
    return (((type * N) + a) * N) + b;
}

static int to_sudoku_box(int row, int col)
{
    return ((row / N_BOX_SIDE) * N_BOX_SIDE) + (col / N_BOX_SIDE);
}

static int to_sudoku_cell(int row_index)
{
    return row_index / N;
}

static int to_sudoku_num(int row_index)
{
    return row_index % N;
}

int solve_dlx_sudoku(const char *problem, char *result)
{
    static dlx_t *dlx;

    memset(result, '.', N_CELL);
    result[N_CELL] = '\0';

    int solved_ploblem = 0;

    dlx = dlx_new((N * N_ROW * N_COL), (N_ROW * N_COL * N_TYPE_COL), solve_dlx_sudoku_cb, result);

    if(dlx == (dlx_t*)0)
    {
        return 0;
    }

    dlx_set_all_cell(dlx);
    set_dlx_sudoku_problem(dlx, problem);

    if(dlx_solve(dlx))
    {
        solved_ploblem = 1;
    }

    dlx_delete(dlx);

    return solved_ploblem;
}

static int solve_dlx_sudoku_cb(int nsolution, int *solutions, void *solved_cb_param)
{
    char *results = (char*)solved_cb_param;
    int solution_i;

    for(solution_i = 0; solution_i < nsolution; solution_i++)
    {
        int dlx_row_index = solutions[solution_i];

        results[to_sudoku_cell(dlx_row_index)] = to_sudoku_num(dlx_row_index) + '1';
    }

    return 1;
}

static void dlx_set_all_cell(dlx_t *dlx)
{
    int num, row, col;

    for(num = 0; num < N; num++)
    {
        for(row = 0; row < N_ROW; row++)
        {
            for(col = 0; col < N_COL; col++)
            {
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(0, row, col));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(1, row, num));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(2, col, num));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(3, to_sudoku_box(row ,col), num));
            }
        }
    }
}

static void set_dlx_sudoku_problem(dlx_t *dlx, const char *problem)
{
    int row, col;

    for(row = 0; row < N_ROW; row++)
    {
        for(col = 0; col < N_COL; col++)
        {
            if(isdigit(*problem) && (*problem != '0'))
            {
                dlx_select_and_remove_row(dlx, to_dlx_row(row, col, (int)(*problem - '1')));
            }

            problem++;
        }
    }
}

void load_and_solve_sudoku(const char *filename)
{
    FILE *fp;
    static char tmp[N_CELL+1];
    static char result[N_CELL+1];
    char *s;

    fp = fopen(filename, "r");

    if(fp != (FILE*)0)
    {
        while((s = fgets(tmp, N_CELL+1, fp)) != (char*)0)
        {
            if(strlen(s) == N_CELL)
            {
                printf(" problem: %s\n", s);

                solve_dlx_sudoku(s, result);

                printf(" result : %s\n", result);
            }
        }

        fclose(fp);
    }
}

#ifdef USE_SUDOKU_MAIN
int main(int argc, char **argv)
{
    load_and_solve_sudoku(PROBLEM_FILE1);
    load_and_solve_sudoku(PROBLEM_FILE2);
    load_and_solve_sudoku(PROBLEM_FILE3);

    return EXIT_SUCCESS;
}
#endif
