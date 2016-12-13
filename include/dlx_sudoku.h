//!
//! @file      dlx_sudoku.h
//! @brief     dlx_sudoku モジュール定義
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

//! @brief  DLXで数独を解く
//! @param  problem 数独の問題 null文字でターミネートされた文字列で、1-9の数字以外は空白とみなす
//! @param  result  数独の解 null文字でターミネートされた文字列で、.は空白を表す
//! @retval 0       数独を解けなかった
//! @retval 1       数独を解けた
int solve_dlx_sudoku(const char *problem, char *result);

#ifdef __cplusplus
}
#endif
