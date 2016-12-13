//!
//! @file      SVMOCR.h
//! @brief     SVMOCR クラス定義
//!

#pragma once

#include <svm.h>

#include "SudokuOCR.h"

#define IMAGE_RC 30 //!< 認識画像のROW, COLサイズ

#define DATA_RC  IMAGE_RC             //!< 認識データのROW, COLサイズ
#define DATA_SIZE (DATA_RC * DATA_RC) //!< 認識データのサイズ

#define NR_CLASS (10) //!< 分類クラス (' ' と '1' - '9' の 10種)

//! @brief SVMを利用して数字を認識するクラス
class SVMOCR: public SudokuOCR
{
    struct svm_model *model; //!< libsvm のモデル

    unsigned char data[DATA_SIZE];  //!< 認識用データ
    struct svm_node x[DATA_SIZE+1]; //!< svm_predict* への入力データ
    double probability[NR_CLASS];   //!< 確度
    int label_to_index[NR_CLASS];   //!< label から probability の index への変換テーブル

public:
    //! @brief  初期化処理
    //! @param  file_name モデルファイル
    //! @retval true      成功
    //! @retval false     失敗
    virtual bool initialize(const char *file_name);

    //! @brief 終了処理
    virtual void finalize(void);

    //! @brief  数字認識処理
    //! @param  mat 認識対象画像
    //! @retval 1-9 認識した数値
    //! @retval 0   空白
    virtual int recognize_number(Mat &mat);

    //! @brief ディレクトリ単位のテスト
    //! @param dir_name テストデータがあるディレクトリ名
    void test_dir(const char *dir_name);

    //! @brief  ファイル単位のテスト
    //! @param  file_name テストファイル名
    //! @retval true      期待値と一致
    //! @retval false     期待値と不一致
    bool test_file(const char *file_name, int label);

private:
    //! @brief イメージの表示 (デバッグ用)
    //! @param mat 表示するイメージ
    void print_image(Mat &mat);

    //! @brief 特徴量の計算 (画像から認識データへの変換)
    //! @param mat  入力画像
    //! @param data 認識データ
    void compute_feature(Mat &mat, unsigned char *data);

    //! @brief 正規化 (高さによる正規化)
    //! @param src 入力画像
    //! @param dst 変換画像
    void normalize(Mat &src, Mat &dst);

    //! @brief  認識処理
    //! @param  data 認識データ
    //! @retval 1-9  認識した数値
    //! @retval 0    空白
    int predict(unsigned char *data);
};
