//!
//! @file      SudokuOCR.h
//! @brief     SudokuOCR インタフェース定義 ファクトリ定義
//!

#pragma once

#include <opencv2/core/core.hpp>

using namespace cv;

//! @brief 数字を認識するインタフェース
class SudokuOCR
{
public:
    //! @brief  初期化処理
    //! @param  file_name libsvmモデルファイル
    //! @retval true      成功
    //! @retval false     失敗
    virtual bool initialize(const char *filename = nullptr) = 0;

    //! @brief  数字認識処理
    //! @param  mat 認識対象画像
    //! @retval 1-9 認識した数値
    //! @retval 0   空白
    virtual int recognize_number(Mat &mat) = 0;

    //! @brief 終了処理
    virtual void finalize(void) = 0;
};

//! @brief  数字を認識するインスタンスを生成する
//! @detail インスタンス使用後はdeleteが必要となる。
//! @param  class_name 生成するクラス名 ["SVMOCR": SVMを利用したクラス]
//! @return 生成したインスタンス
SudokuOCR *sudokuOCRFactory(const char *class_name);
