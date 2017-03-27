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

//! @brief DLX構造体型
typedef struct dlx_s dlx_t;

//! @brief  DLXで解が得られたときのコールバック関数型
//! @param  nsolution 解の数
//! @param  solutions 解のリスト
//! @param  solved_cb_param コールバック関数の引数
//! @retval 0 解として適切でない場合
//! @retval 1 解として適切な場合
typedef int (*dlx_solved_cb_t)(int nsolution, int *solutions, void *solved_cb_param);

//! @brief  DLX構造体を動的に作成する
//! @param  nrow            行の数
//! @param  ncol            列の数
//! @param  solved_cb       解が得られたときのコールバック関数
//! @param  solved_cb_param コールバック関数の引数
//! @retval NULL   作成に失敗した場合
//! @return others 作成した構造体
dlx_t *dlx_new(int nrow, int ncol, dlx_solved_cb_t solved_cb, void *solved_cb_param);

//! @brief 動的に作成したDLX構造体を破棄する
//! @param dlx 破棄する構造体
void dlx_delete(dlx_t *dlx);

//! @brief  DLXの要素を配置する
//! @param  dlx 使用するDLX構造体
//! @param  row 配置する行
//! @param  col 配置する列
//! @retval 0   配置に失敗した場合
//! @retval 1   配置に成功した場合
int dlx_set_cell(dlx_t *dlx, int row, int col);

//! @brief DLXの行を選択して要素の所属する列を削除する
void dlx_select_and_remove_row(dlx_t *dlx, int row_index);

//! @brief  DLXで問題を解く
//! @param  dlx 使用するDLX構造体
//! @retval 0 解が得られなかった場合
//! @retval 1 解が得られた場合
int dlx_solve(dlx_t *dlx);

#ifdef __cplusplus
}
#endif
