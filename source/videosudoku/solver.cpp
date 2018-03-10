//!
//! @file      dlx_sudoku.c
//! @brief     dlx_sudoku モジュール実装
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#include "dlx_sudoku.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "dlx.h"

#define N 9     //!< 数独の数字の種類
#define N_ROW N //!< 数独の行の数
#define N_COL N //!< 数独の列の数
#define N_BOX N //!< 数独のボックスのマスの数

#define N_BOX_SIDE 3         //!< 数独のボックスの1辺のマスの数
#define N_CELL N_ROW * N_COL //!< 数独のマスの数
#define N_TYPE_COL 4         //!< 数独の条件の種類数

#define PROBLEM_FILE1 "resource/test/top95.txt"   //!< テスト用問題ファイル1
#define PROBLEM_FILE2 "resource/test/diff.txt"    //!< テスト用問題ファイル2
#define PROBLEM_FILE3 "resource/test/hardest.txt" //!< テスト用問題ファイル3

//! @brief  数独のマスの位置と数字をDLXの行に変換する
//! @param  row マスの行
//! @param  col マスの列
//! @param  num 数字
//! @return DLXの行
static int to_dlx_row(const int row, const int col, const int num)
{
    return (((row * N) + col) * N) + num;
}

//! @brief  数独の条件をDLXの列に変換する
//! @param  type 条件の種類
//! @param  a    条件の要素a
//! @param  b    条件の要素b
//! @return DLXの列
static int to_dlx_col(const int type, const int a, const int b)
{
    return (((type * N) + a) * N) + b;
}

//! @brief  数独のマスの所属するボックスを計算する
//! @param  row マスの行
//! @param  col マスの列
//! @return ボックスの番号
static int to_sudoku_box(const int row, const int col)
{
    return (row / N_BOX_SIDE * N_BOX_SIDE) + (col / N_BOX_SIDE);
}

//! @brief  DLXの行を数独の配列のインデックスに変換する
//! @param  row_index 行
//! @return 配列のインデックス
static int to_sudoku_cell(const int row_index)
{
    return row_index / N;
}

//! @brief  DLXの行を数独の数字に変換する
//! @param  row_index 行
//! @return 数字
static int to_sudoku_num(const int row_index)
{
    return row_index % N;
}

//! @brief  DLXで解いた数独の解を配列に詰める
//! @param  nsolution       DLXでの解の数
//! @param  solutions       DLXでの解の配列
//! @param  solved_cb_param 数独の解の配列
//! @return 常に1を返す
static int solve_dlx_sudoku_cb(int nsolution, int *solutions, void *solved_cb_param)
{
    char *results = (char*)solved_cb_param;

    for(int solution_i = 0; solution_i < nsolution; ++solution_i)
    {
        const int dlx_row_index = solutions[solution_i];

        results[to_sudoku_cell(dlx_row_index)] = (char)(to_sudoku_num(dlx_row_index) + '1');
    }

    return 1;
}

//! @brief 数独用にDLXの全要素を配置する
//! @param dlx 要素を配置するDLX構造体
static void dlx_set_all_cell(dlx_t *dlx)
{
    for(int num = 0; num < N; ++num)
    {
        for(int row = 0; row < N_ROW; ++row)
        {
            for(int col = 0; col < N_COL; ++col)
            {
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(0, row, col));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(1, row, num));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(2, col, num));
                dlx_set_cell(dlx, to_dlx_row(row, col, num), to_dlx_col(3, to_sudoku_box(row, col), num));
            }
        }
    }
}

//! @brief DLXに数独の問題を設定する
//! @param dlx     数独用に全要素が配置されたDLX構造体
//! @param problem 数独の問題
static void set_dlx_sudoku_problem(dlx_t *dlx, const char *problem)
{
    for(int row = 0; row < N_ROW; ++row)
    {
        for(int col = 0; col < N_COL; ++col)
        {
            if(isdigit(*problem) && (*problem != '0'))
            {
                dlx_select_and_remove_row(dlx, to_dlx_row(row, col, (int)(*problem - '1')));
            }

            ++problem;
        }
    }
}

int solve_dlx_sudoku(const char *problem, char *result)
{
    dlx_t *dlx = dlx_new(N * N_ROW * N_COL, N_ROW * N_COL * N_TYPE_COL, solve_dlx_sudoku_cb, result);

    if(dlx == NULL) return 0;

    memset(result, '.', N_CELL);

    result[N_CELL] = '\0';

    int solved_ploblem = 0;

    dlx_set_all_cell(dlx);
    set_dlx_sudoku_problem(dlx, problem);

    if(dlx_solve(dlx))
    {
        solved_ploblem = 1;
    }

    dlx_delete(dlx);

    return solved_ploblem;
}

#ifdef DLX_SUDOKU_MAIN
//! @brief ファイルから数独の問題を読んでそれを解く
//! @param filename ファイルのパス
static void load_and_solve_sudoku(const char *filename)
{
    char problem[N_CELL + 1] = {0};
    char result[N_CELL + 1] = {0};

    FILE *fp = fopen(filename, "r");

    if(fp == NULL) return;

    while(fgets(problem, N_CELL + 1, fp) != NULL)
    {
        if(strlen(problem) != N_CELL) continue;

        solve_dlx_sudoku(problem, result);

        printf(" problem: %s\n", problem);
        printf(" result : %s\n", result);
    }

    fclose(fp);
}

int main(void)
{
    load_and_solve_sudoku(PROBLEM_FILE1);
    load_and_solve_sudoku(PROBLEM_FILE2);
    load_and_solve_sudoku(PROBLEM_FILE3);

    return 0;
}
#endif
