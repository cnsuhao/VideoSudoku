//!
//! @file  SVMOCR.h
//! @brief SVMOCR クラス定義
//!

#pragma once

#include <svm.h>

#include "SudokuOCR.h"

namespace videosudoku
{
constexpr auto IMAGE_RC = 30; //!< 認識画像のROW, COLサイズ

constexpr auto DATA_RC = IMAGE_RC;            //!< 認識データのROW, COLサイズ
constexpr auto DATA_SIZE = DATA_RC * DATA_RC; //!< 認識データのサイズ

constexpr auto NR_CLASS = 10; //!< 分類クラス (' ' と '1' - '9' の 10種)

//! @brief SVMを利用して数字を認識するクラス
class SVMOCR final: public SudokuOCR
{
public:
    virtual bool initialize(const char *file_name) override;
    virtual int recognize_number(cv::Mat &mat) override;
    virtual void finalize() override;

private:
    //! @brief 特徴量の計算 (画像から認識データへの変換)
    //! @param mat  入力画像
    //! @param data 認識データ
    void compute_feature(cv::Mat &mat, unsigned char *data) const;

    //! @brief 正規化 (高さによる正規化)
    //! @param src 入力画像
    //! @param dst 変換画像
    void normalize(const cv::Mat &src, cv::Mat &dst) const;

    //! @brief  認識処理
    //! @param  data 認識データ
    //! @retval 1-9  認識した数値
    //! @retval 0    空白
    int predict(unsigned char *data);

    svm_model *model = nullptr; //!< libsvm のモデル

    unsigned char data[DATA_SIZE] = {0}; //!< 認識用データ

    svm_node x[DATA_SIZE + 1];      //!< svm_predict* への入力データ

    double probability[NR_CLASS] = {0}; //!< 確度
    int label_to_index[NR_CLASS] = {0}; //!< label から probability の index への変換テーブル
};
}
