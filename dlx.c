//!
//! @file      dlx.c
//! @brief     dlx モジュール実装
//! @author    Yamato Komei
//! @date      2016/8/5
//! @copyright (c) 2016 Yamato Komei
//!

#include <stdlib.h>
#include <memory.h>

#include "dlx.h"

struct dlx_cell_s;

typedef struct dlx_cell_s dlx_cell_t;

struct dlx_cell_s
{
    int row_index;

    dlx_cell_t *up;
    dlx_cell_t *down;
    dlx_cell_t *right;
    dlx_cell_t *left;

    int nrow;
    dlx_cell_t *column_header;
};

struct dlx_s
{
    dlx_cell_t *root;
    int ncol;
    dlx_cell_t *column_headers;
    int nrow;
    dlx_cell_t **row_pointers;

    int nresult;
    int *results;

    dlx_solved_cb_t solved_cb;
    void *solved_cb_param;
};

static dlx_t *dlx_allocate(int nrow, int ncol);
static void dlx_initialize_work(dlx_t *dlx);
static void dlx_free_all_cells(dlx_t *dlx);
static dlx_cell_t *dlx_choose_column(dlx_t *dlx);
static void dlx_remove_column(dlx_t *dlx, dlx_cell_t *column_header);
static void dlx_restore_column(dlx_t *dlx, dlx_cell_t *column_header);
static void dlx_initialize_column_headers(dlx_t *dlx);

static inline void dlx_cell_up_down_self(dlx_cell_t *cell)
{
    cell->up = cell->down = cell;
}

static inline void dlx_cell_left_right_self(dlx_cell_t *cell)
{
    cell->left = cell->right = cell;
}

static inline void dlx_cell_remove_up_down(dlx_cell_t *cell)
{
    cell->down->up = cell->up;
    cell->up->down = cell->down;
    cell->column_header->nrow--;
}

static inline void dlx_cell_remove_left_right(dlx_cell_t *cell)
{
    cell->right->left = cell->left;
    cell->left->right = cell->right;
}

static inline void dlx_cell_restore_up_down(dlx_cell_t *cell)
{
    cell->down->up = cell;
    cell->up->down = cell;
    cell->column_header->nrow++;
}

static inline void dlx_cell_restore_left_right(dlx_cell_t *cell)
{
    cell->right->left = cell;
    cell->left->right = cell;
}

static inline void dlx_add_column(dlx_cell_t *head_cell, dlx_cell_t *new_cell)
{
    new_cell->up = head_cell->up;
    new_cell->down = head_cell;
    head_cell->up->down = new_cell;
    head_cell->up = new_cell;
    new_cell->column_header->nrow++;
}

static inline void dlx_add_row(dlx_cell_t *head_cell, dlx_cell_t *new_cell)
{
    new_cell->left  = head_cell->left;
    new_cell->right = head_cell;
    head_cell->left->right = new_cell;
    head_cell->left = new_cell;
}

static inline void dlx_push_result(dlx_t *dlx, int result)
{
    dlx->results[dlx->nresult++] = result;
}

static inline void dlx_unpush_result(dlx_t *dlx)
{
    dlx->nresult--;
}

static inline void dlx_clear_results(dlx_t *dlx)
{
    dlx->nresult = 0;
}

dlx_t *dlx_new(int nrow, int ncol, dlx_solved_cb_t solved_cb, void *solved_cb_param)
{
    dlx_t *dlx = dlx_allocate(nrow, ncol);

    if(dlx == (dlx_t*)0)
    {
        return (dlx_t*)0;
    }

    dlx->nrow = nrow;
    dlx->ncol = ncol;
    dlx->solved_cb = solved_cb;
    dlx->solved_cb_param = solved_cb_param;

    dlx_initialize_work(dlx);

    return dlx;
}

static dlx_t *dlx_allocate(int nrow, int ncol)
{
    dlx_t *dlx = malloc(sizeof(dlx_t));

    if(dlx == (dlx_t*)0)
    {
        return (dlx_t*)0;
    }

    memset(dlx, 0, sizeof(*dlx));

    dlx->column_headers = malloc(sizeof(dlx_cell_t) * ncol);

    if(dlx->column_headers == (dlx_cell_t*)0)
    {
        free(dlx);

        return (dlx_t*)0;
    }

    dlx->row_pointers = malloc(sizeof(dlx_cell_t*) * nrow);

    if(dlx->row_pointers == (dlx_cell_t**)0)
    {
        free(dlx->column_headers);
        free(dlx);

        return (dlx_t*)0;
    }

    dlx->results = malloc(sizeof(int) * nrow);

    if(dlx->results == (int*)0)
    {
        free(dlx->column_headers);
        free(dlx->row_pointers);
        free(dlx);

        return (dlx_t*)0;
    }

    dlx->root = malloc(sizeof(dlx_cell_t));

    if(dlx->root == (dlx_cell_t*)0)
    {
        free(dlx->column_headers);
        free(dlx->row_pointers);
        free(dlx->results);
        free(dlx);

        return (dlx_t*)0;
    }

    return dlx;
}

static void dlx_initialize_work(dlx_t *dlx)
{
    memset(dlx->row_pointers, 0, sizeof(dlx_cell_t*) * dlx->nrow);
    memset(dlx->root, 0, sizeof(dlx_cell_t));
    dlx_cell_left_right_self(dlx->root);
    dlx_cell_up_down_self(dlx->root);

    dlx_initialize_column_headers(dlx);

    dlx_clear_results(dlx);
}

static void dlx_initialize_column_headers(dlx_t *dlx)
{
    int col_i;

    for(col_i = 0; col_i < dlx->ncol; col_i++)
    {
        dlx_cell_up_down_self(&dlx->column_headers[col_i]);
        dlx_add_row(dlx->root, &dlx->column_headers[col_i]);
        dlx->column_headers[col_i].nrow = 0;
        dlx->column_headers[col_i].row_index = dlx->nrow + 1;
    }
}

void dlx_delete(dlx_t *dlx)
{
    dlx_free_all_cells(dlx);

    free(dlx->row_pointers);
    free(dlx->column_headers);
    free(dlx->results);
    free(dlx->root);
    free(dlx);
}

static void dlx_free_all_cells(dlx_t *dlx)
{
    int row_i;

    dlx_cell_t *head_cell;
    dlx_cell_t *next_cell;
    dlx_cell_t *cell;

    for(row_i = 0; row_i < (dlx->nrow); row_i++)
    {
        head_cell = dlx->row_pointers[row_i];

        if(head_cell)
        {
            cell = head_cell->right;

            while(head_cell != cell)
            {
                next_cell = cell->right;
                free(cell);
                cell = next_cell;
            }

            free(head_cell);
        }
    }
}

int dlx_set_cell(dlx_t *dlx, int row, int col)
{
    dlx_cell_t *cell = malloc(sizeof(dlx_cell_t));

    if(cell == (dlx_cell_t*)0)
    {
        return 0;
    }

    cell->row_index = row;
    cell->column_header = &dlx->column_headers[col];

    dlx_add_column(cell->column_header, cell);

    if(dlx->row_pointers[row] == (dlx_cell_t*)0)
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
    if(dlx->row_pointers[row_index] == (dlx_cell_t*)0)
    {
        return;
    }

    dlx_cell_t *cell = dlx->row_pointers[row_index];

    do
    {
        dlx_remove_column(dlx, cell->column_header);
        cell = cell->right;
    }
    while(cell != dlx->row_pointers[row_index]);

    dlx_push_result(dlx, row_index);
}

int dlx_solve(dlx_t *dlx)
{
    dlx_cell_t *select_column;
    dlx_cell_t *select_row;
    dlx_cell_t *r_column;

    if(dlx->root->right == dlx->root)
    {
        if(dlx->solved_cb)
        {
            return dlx->solved_cb(dlx->nresult, dlx->results, dlx->solved_cb_param);
        }

        return 0;
    }

    select_column = dlx_choose_column(dlx);

    if(select_column == (dlx_cell_t*)0)
    {
        return 0;
    }

    select_row = select_column->down;
    dlx_remove_column(dlx ,select_column);

    while(select_row != select_column)
    {
        dlx_push_result(dlx, select_row->row_index);
        r_column = select_row->right;

        while(r_column != select_row)
        {
            dlx_remove_column(dlx ,r_column->column_header);
            r_column = r_column->right;
        }

        if(dlx_solve(dlx))
        {
            return 1;
        }

        dlx_unpush_result(dlx);

        // rightよりleftの方が速かった。
        r_column = select_row->left;

        while(r_column != select_row)
        {
            dlx_restore_column(dlx, r_column->column_header);
            r_column = r_column->left;
        }

        select_row = select_row->down;
    }

    dlx_restore_column(dlx, select_column);

    return 0;
}

dlx_cell_t *dlx_choose_column(dlx_t *dlx)
{
    int min_nrow = dlx->root->right->nrow;
    dlx_cell_t *select_column = dlx->root->right;
    dlx_cell_t *column_cell = dlx->root->right;

    while(column_cell != dlx->root)
    {
        if(column_cell->nrow == 0)
        {
            return (dlx_cell_t*)0;
        }

        if(min_nrow > column_cell->nrow)
        {
            min_nrow = column_cell->nrow;
            select_column = column_cell;
        }

        column_cell = column_cell->right;
    }

    return select_column;
}

void dlx_remove_column(dlx_t *dlx, dlx_cell_t *column_header)
{
    dlx_cell_remove_left_right(column_header);

    dlx_cell_t *row_cell;
    dlx_cell_t *column_cell = column_header->down;

    while(column_cell != column_header)
    {
        row_cell = column_cell->right;

        while(row_cell != column_cell)
        {
            dlx_cell_remove_up_down(row_cell);
            row_cell = row_cell->right;
        }

        column_cell = column_cell->down;
    }
}

void dlx_restore_column(dlx_t *dlx, dlx_cell_t *column_header)
{
    dlx_cell_t *row_cell;
    dlx_cell_t *column_cell = column_header->up;

    while(column_cell != column_header)
    {
        row_cell = column_cell->left;

        while(row_cell != column_cell)
        {
            dlx_cell_restore_up_down(row_cell);
            row_cell = row_cell->left;
        }

        column_cell = column_cell->up;
    }

    dlx_cell_restore_left_right(column_header);
}
