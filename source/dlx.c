//!
//! @file      dlx.c
//! @brief     dlx モジュール実装
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#include "dlx.h"

#include <memory.h>
#include <stdlib.h>

struct dlx_cell_s;

//! @brief DLXの要素構造体型
typedef struct dlx_cell_s dlx_cell_t;

//! @brief DLXの構造体
struct dlx_s
{
    int ncol;    //!< 列の数
    int nrow;    //!< 行の数
    int nresult; //!< 解の数

    int *results; //!< 解のスタック

    dlx_cell_t *root;           //!< ルート
    dlx_cell_t *column_headers; //!< 列ヘッダの配列

    dlx_cell_t **row_pointers; //!< 行ポインタの配列

    dlx_solved_cb_t solved_cb; //!< 解が得られたときに呼ぶコールバック関数

    void *solved_cb_param; //!< コールバック関数の引数
};

//! @brief DLXの要素の構造体
struct dlx_cell_s
{
    int row_index; //!< 行
    int nrow;      //!< 自分が列ヘッダのときに列に所属する要素の数

    dlx_cell_t *column_header; //!< 所属する列ヘッダ
    dlx_cell_t *up;            //!< 上の要素
    dlx_cell_t *down;          //!< 下の要素
    dlx_cell_t *right;         //!< 右の要素
    dlx_cell_t *left;          //!< 左の要素
};

//! @brief 上下を自分自身とつなぐ
//! @param cell 対象の要素
static void dlx_cell_up_down_self(dlx_cell_t *cell)
{
    cell->up = cell->down = cell;
}

//! @brief 左右を自分自身とつなぐ
//! @param cell 対象の要素
static void dlx_cell_left_right_self(dlx_cell_t *cell)
{
    cell->left = cell->right = cell;
}

//! @brief 列から要素を切り離す
//! @param cell 切り離す要素
static void dlx_cell_remove_up_down(dlx_cell_t *cell)
{
    cell->down->up = cell->up;
    cell->up->down = cell->down;

    --cell->column_header->nrow;
}

//! @brief 行から要素を切り離す
//! @param cell 切り離す要素
static void dlx_cell_remove_left_right(dlx_cell_t *cell)
{
    cell->right->left = cell->left;
    cell->left->right = cell->right;
}

//! @brief 列に要素を戻す
//! @param cell 戻す要素
static void dlx_cell_restore_up_down(dlx_cell_t *cell)
{
    cell->down->up = cell;
    cell->up->down = cell;

    ++cell->column_header->nrow;
}

//! @brief 行に要素を戻す
//! @param cell 戻す要素
static void dlx_cell_restore_left_right(dlx_cell_t *cell)
{
    cell->right->left = cell;
    cell->left->right = cell;
}

//! @brief 列に要素を追加する
//! @param head_cell 追加する列のヘッダ
//! @param new_cell  追加する要素
static void dlx_add_column(dlx_cell_t *head_cell, dlx_cell_t *new_cell)
{
    new_cell->up = head_cell->up;
    new_cell->down = head_cell;
    head_cell->up->down = new_cell;
    head_cell->up = new_cell;

    ++new_cell->column_header->nrow;
}

//! @brief 行に要素を追加する
//! @param head_cell 追加する行のヘッダ
//! @param new_cell  追加する要素
static void dlx_add_row(dlx_cell_t *head_cell, dlx_cell_t *new_cell)
{
    new_cell->left = head_cell->left;
    new_cell->right = head_cell;
    head_cell->left->right = new_cell;
    head_cell->left = new_cell;
}

//! @brief 解を一つスタックに追加する
//! @param dlx    解を追加するDLX構造体
//! @param result 追加する解
static void dlx_push_result(dlx_t *dlx, const int result)
{
    dlx->results[dlx->nresult++] = result;
}

//! @brief 解を一つスタックから除去する
//! @param dlx 解を除去するDLX構造体
static void dlx_unpush_result(dlx_t *dlx)
{
    --dlx->nresult;
}

//! @brief スタックから解を消去する
//! @param dlx 解を消去するDLX構造体
static void dlx_clear_results(dlx_t *dlx)
{
    dlx->nresult = 0;
}

//! @brief  DLX構造体を確保する
//! @param  nrow DLXの行数
//! @param  ncol DLXの列数
//! @retval NULL   確保できなかった場合
//! @retval others 確保した構造体
static dlx_t *dlx_allocate(const int nrow, const int ncol)
{
    dlx_t *dlx = malloc(sizeof(dlx_t));

    if(dlx == NULL) return NULL;

    dlx->results = malloc(sizeof(int) * nrow);
    dlx->root = malloc(sizeof(dlx_cell_t));
    dlx->column_headers = malloc(sizeof(dlx_cell_t) * ncol);
    dlx->row_pointers = malloc(sizeof(dlx_cell_t*) * nrow);

    if(dlx->root == NULL || dlx->column_headers == NULL || dlx->row_pointers == NULL || dlx->results == NULL) dlx_delete(dlx);

    return dlx;
}

//! @brief 列ヘッダを全て初期化する
//! @param dlx 列ヘッダを初期化するDLX構造体
static void dlx_initialize_column_headers(dlx_t *dlx)
{
    for(int col_i = 0; col_i < dlx->ncol; ++col_i)
    {
        dlx_cell_up_down_self(&dlx->column_headers[col_i]);
        dlx_add_row(dlx->root, &dlx->column_headers[col_i]);

        dlx->column_headers[col_i].nrow = 0;
        dlx->column_headers[col_i].row_index = dlx->nrow + 1;
    }
}

//! @brief DLXを初期化する
//! @param dlx 初期化するDLX構造体
static void dlx_initialize_work(dlx_t *dlx)
{
    for(int row_i = 0; row_i < dlx->nrow; ++row_i)
    {
        dlx->row_pointers[row_i] = NULL;
    }

    dlx_cell_left_right_self(dlx->root);
    dlx_cell_up_down_self(dlx->root);

    dlx_initialize_column_headers(dlx);

    dlx_clear_results(dlx);
}

//! @brief 全ての要素を解放する
//! @param dlx 要素を解放するDLX構造体
static void dlx_free_all_cells(dlx_t *dlx)
{
    for(int row_i = 0; row_i < dlx->nrow; ++row_i)
    {
        dlx_cell_t *head_cell = dlx->row_pointers[row_i];

        if(head_cell == NULL) continue;

        dlx_cell_t *cell = head_cell->right;

        while(head_cell != cell)
        {
            dlx_cell_t *next_cell = cell->right;

            free(cell);

            cell = next_cell;
        }

        free(head_cell);
    }
}

//! @brief  要素の一番少ない列を選ぶ
//! @param  dlx    使用するDLX構造体
//! @retval NULL   要素の存在しない列があった場合
//! @retval others 選んだ列のヘッダ
static dlx_cell_t *dlx_choose_column(dlx_t *dlx)
{
    int min_nrow = dlx->root->right->nrow;

    dlx_cell_t *column_cell = dlx->root->right;
    dlx_cell_t *select_column = dlx->root->right;

    while(column_cell != dlx->root)
    {
        if(column_cell->nrow == 0) return NULL;

        if(min_nrow > column_cell->nrow)
        {
            min_nrow = column_cell->nrow;
            select_column = column_cell;
        }

        column_cell = column_cell->right;
    }

    return select_column;
}

//! @brief 列を行から切り離す
//! @param column_header 切り離す列のヘッダ
static void dlx_remove_column(dlx_cell_t *column_header)
{
    dlx_cell_remove_left_right(column_header);

    dlx_cell_t *column_cell = column_header->down;

    while(column_cell != column_header)
    {
        dlx_cell_t *row_cell = column_cell->right;

        while(row_cell != column_cell)
        {
            dlx_cell_remove_up_down(row_cell);

            row_cell = row_cell->right;
        }

        column_cell = column_cell->down;
    }
}

//! @brief 列を行に戻す
//! @param column_header 戻す列のヘッダ
static void dlx_restore_column(dlx_cell_t *column_header)
{
    dlx_cell_t *column_cell = column_header->up;

    while(column_cell != column_header)
    {
        dlx_cell_t *row_cell = column_cell->left;

        while(row_cell != column_cell)
        {
            dlx_cell_restore_up_down(row_cell);

            row_cell = row_cell->left;
        }

        column_cell = column_cell->up;
    }

    dlx_cell_restore_left_right(column_header);
}

dlx_t *dlx_new(int nrow, int ncol, dlx_solved_cb_t solved_cb, void *solved_cb_param)
{
    dlx_t *dlx = dlx_allocate(nrow, ncol);

    if(dlx == NULL) return NULL;

    dlx->nrow = nrow;
    dlx->ncol = ncol;
    dlx->solved_cb = solved_cb;
    dlx->solved_cb_param = solved_cb_param;

    dlx_initialize_work(dlx);

    return dlx;
}

void dlx_delete(dlx_t *dlx)
{
    dlx_free_all_cells(dlx);

    free(dlx->results);
    free(dlx->column_headers);
    free(dlx->row_pointers);
    free(dlx->root);
    free(dlx);
}

int dlx_set_cell(dlx_t *dlx, int row, int col)
{
    dlx_cell_t *cell = malloc(sizeof(dlx_cell_t));

    if(cell == NULL) return 0;

    cell->row_index = row;
    cell->column_header = &dlx->column_headers[col];

    dlx_add_column(cell->column_header, cell);

    if(dlx->row_pointers[row] == NULL)
    {
        dlx_cell_left_right_self(cell);

        dlx->row_pointers[row] = cell;
    }
    else
    {
        dlx_add_row(dlx->row_pointers[row], cell);
    }

    return 1;
}

void dlx_select_and_remove_row(dlx_t *dlx, int row_index)
{
    if(dlx->row_pointers[row_index] == NULL) return;

    dlx_cell_t *cell = dlx->row_pointers[row_index];

    do
    {
        dlx_remove_column(cell->column_header);

        cell = cell->right;
    }
    while(cell != dlx->row_pointers[row_index]);

    dlx_push_result(dlx, row_index);
}

int dlx_solve(dlx_t *dlx)
{
    if(dlx->root->right == dlx->root)
    {
        if(dlx->solved_cb == NULL) return 0;

        return dlx->solved_cb(dlx->nresult, dlx->results, dlx->solved_cb_param);
    }

    dlx_cell_t *select_column = dlx_choose_column(dlx);

    if(select_column == NULL) return 0;

    dlx_cell_t *select_row = select_column->down;

    dlx_remove_column(select_column);

    while(select_row != select_column)
    {
        dlx_push_result(dlx, select_row->row_index);

        dlx_cell_t *r_column = select_row->right;

        while(r_column != select_row)
        {
            dlx_remove_column(r_column->column_header);

            r_column = r_column->right;
        }

        if(dlx_solve(dlx) == 1) return 1;

        dlx_unpush_result(dlx);

        dlx_cell_t *l_column = select_row->left;

        while(l_column != select_row)
        {
            dlx_restore_column(l_column->column_header);

            l_column = l_column->left;
        }

        select_row = select_row->down;
    }

    dlx_restore_column(select_column);

    return 0;
}
