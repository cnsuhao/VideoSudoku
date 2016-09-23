//!
//! @file      dlx.h
//! @brief     dlx モジュール定義
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

struct dlx_s;

typedef struct dlx_s dlx_t;
typedef int (*dlx_solved_cb_t)(int, int*, void*);

dlx_t *dlx_new(int nrow, int ncol, dlx_solved_cb_t solved_cb, void *solved_cb_param);
void dlx_delete(dlx_t *dlx);

int dlx_set_cell(dlx_t *dlx, int row, int col);
void dlx_select_and_remove_row(dlx_t *dlx, int row_index);
int dlx_solve(dlx_t *dlx);

#ifdef __cplusplus
}
#endif
