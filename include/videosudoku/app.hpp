//!
//! @file      VideoSudoku.h
//! @brief     VideoSudoku クラス定義
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#pragma once

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "debuglog.h"
#include "SudokuOCR.h"

namespace videosudoku
{
//! @brief カメラからの入力画像から数独を検出して、その解をリアルタイムに表示するクラス
class VideoSudoku final
{
public:
    //! @brief コンストラクタ
    VideoSudoku();

    //! @brief デストラクタ
    ~VideoSudoku();

    //! @brief  初期化処理
    //! @param  size      結果画像のサイズ
    //! @parem  device_id 使用するカメラデバイスのID
    //! @retval 0         正常終了
    //! @retval 1         カメラデバイスを開けなかった
    //! @retval 2         文字認識オブジェクトの初期化失敗
    //! @retval 3         モデルデータの読み込み失敗
    int initialize(int size, int device_id);

    //! @brief 終了処理
    void finalize();

    //! @brief  ビデオ入力を取得
    //! @retval true  成功
    //! @retval false 失敗
    bool capture_video();

    //! @brief 画面表示
    //! @param results_availability 結果を表示する場合:true 表示しない場合:false
    void display(bool results_availability);

    //! @brief  画像中の数独を解く
    //! @retval true  数独を解けた
    //! @retval false 数独を解けなかった
    bool solve();

private:
    //! @brief 画像を初期化する
    //! @param frame 初期化する画像
    //! @param size  初期化後のサイズ
    void frame_initialize(cv::Mat &frame, int size) const;

    //! @brief  抽出した輪郭が数独の輪郭として適切であるかの判定
    //! @retval true  適切である
    //! @retval false 適切でない
    bool is_sudoku_contour();

    //! @brief 数独を解いた結果を結果画像に書き込む
    void draw_result();

    //! @brief 数独の枠線を結果画像に書き込む
    void draw_cell();

    //! @brief  画像の歪み補正
    //! @retval true  画像から数独を検出できた
    //! @retval false 画像から数独を検出できなかった
    bool fix_outer_frame();

    //! @brief  画像から数字を認識
    //! @retval true  認識したデータが数独である
    //! @retval false 認識したデータが数独でない
    bool recognize_number();

    //! @brief 画像を二値化する
    //! @param target_frame   処理対象画像
    //! @param threshold_type 反転する場合:THRESH_BINARY_INV しない場合:THRESH_BINARY
    void make_binary_frame(cv::Mat &target_frame, int threshold_type) const;

    //! @brief  画像から数独の輪郭を抽出して直線近似する
    //! @retval true  輪郭を抽出できた
    //! @retval false 輪郭をできなかった
    bool get_outer_contour();

    //! @brief  ホモグラフィー行列を取得する
    //! @param  contour    変換元座標平面の４点
    //! @param  bound_rect 変換先座標平面の４点
    //! @return 取得したホモグラフィー行列
    cv::Mat get_homography(const std::vector<cv::Point> &contour, const cv::Rect &bound_rect) const;

    //! @brief 画像から枠線を消す
    void delete_grid();

    //! @brief  数独を解く
    //! @retval true  数独を解けた
    //! @retval false 数独を解けなかった
    bool sudoku_solve();

    int result_size = 0;  //!< 結果画像の一辺の長さ
    int cell_size = 0;    //!< マスの一辺の長さ
    int text_offset = 0;  //!< 数字表示位置のオフセット

    char *input_problem = nullptr;  //!< 数独の初期値 1-9以外は空白や未定
    char *result_problem = nullptr; //!< 数独の解答結果 1-9以外は空白や未定

    bool initialized = false; //!< オブジェクトが正しく初期化できたかどうか

    SudokuOCR *ocr = nullptr; //!< 文字認識部オブジェクト

    cv::VideoCapture capture; //!< ビデオ入力オブジェクト

    cv::Mat input_frame;  //!< 入力画像
    cv::Mat temp_frame;   //!< 作業用画像
    cv::Mat result_frame; //!< 結果画像

    std::vector<cv::Point> contour; //!< 直線近似した数独の輪郭の頂点データ
};
}