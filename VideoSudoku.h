//!
//! @file      VideoSudoku.h
//! @brief     VideoSudoku クラス定義
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>

#include "SudokuOCR.h"
#include "debuglog.h"

using namespace cv;
using namespace std;

//! @brief カメラからの入力画像から数独を検出して、その解をリアルタイムに表示するクラス
class VideoSudoku
{
public:
    //! @brief コンストラクタ
    VideoSudoku(void);

    //! @brief デストラクタ
    ~VideoSudoku(void);

    //! @brief  初期化処理
    //! @param  size      結果画像のサイズ 最低サイズは400
    //! @parem  device_id 使用するカメラデバイスのID
    //! @retval 0         正常終了
    //! @retval 1         カメラデバイスを開けなかった
    //! @retval 2         文字認識オブジェクトの初期化失敗
    //! @retval 3         モデルデータの読み込み失敗
    int initialize(int size = 400, int device_id = 0);

    //! @brief 終了処理
    void finalize(void);

    //! @brief  ビデオ入力を取得
    //! @retval true  成功
    //! @retval false 失敗
    bool capture_video(void);

    //! @brief 画面表示
    //! @param results_availability 結果を表示する場合:true 表示しない場合:false
    void display(bool results_availability);

    //! @brief  画像の歪み補正
    //! @retval true  画像から数独を検出できた
    //! @retval false 画像から数独を検出できなかった
    bool fix_outer_frame(void);

    //! @brief  画像中の数独を解く
    //! @retval true  数独を解けた
    //! @retval false 数独を解けなかった
    bool solve(void);

private:
    const int cells_number;    //!< 一辺のマスの数
    const int result_min_size; //!< 結果画像の一辺の長さの最小値

    const long input_max_area; //!< 入力画像の面積の最大値

    const char *input_name;  //!< 入力画像ウィンドウの名前
    const char *temp_name;   //!< 作業用画像ウィンドウの名前
    const char *result_name; //!< 結果画像ウィンドウの名前
    const char *ocr_type;    //!< 文字認識オブジェクトの種類
    const char *model;       //!< 文字認識に使うモデルデータのパス

    const Scalar contour_line_color;     //!< 輪郭線色
    const Scalar frame_background_color; //!< 画像の背景色
    const Scalar initial_text_color;     //!< 数独初期値の文字色
    const Scalar result_text_color;      //!< 数独を解いた結果の文字色
    const Scalar cell_line_color;        //!< 数独を解いた結果の枠線色

    int result_size;  //!< 結果画像の一辺の長さ
    int cell_size;    //!< マスの一辺の長さ
    int text_offset;  //!< 数字表示位置のオフセット
    int rotate_phase; //!< 認識時の回転角度 [0:0 1:90 2:180 3:270]

    char *input_problem;  //!< 数独の初期値 1-9以外は空白や未定
    char *result_problem; //!< 数独の解答結果 1-9以外は空白や未定

    bool initialized; //!< オブジェクトが正しく初期化できたかどうか

    SudokuOCR *ocr; //!< 文字認識部オブジェクト

    VideoCapture capture; //!< ビデオ入力オブジェクト

    Mat input_frame;       //!< 入力画像
    Mat temp_frame;        //!< 作業用画像
    Mat result_frame;      //!< 結果画像
    vector<Point> contour; //!< 直線近似した数独の輪郭の頂点データ

    //! @brief 画像を初期化する
    //! @param frame 初期化する画像
    //! @param size  初期化後のサイズ
    void frame_initialize(Mat &frame, int size);

    //! @brief  抽出した輪郭が数独の輪郭として適切であるかの判定
    //! @retval true  適切である
    //! @retval false 適切でない
    bool is_sudoku_contour(void);

    //! @brief 数独を解いた結果を結果画像に書き込む
    void draw_result(void);

    //! @brief 数独の枠線を結果画像に書き込む
    void draw_cell(void);

    //! @brief  画像から数字を認識
    //! @retval true  認識したデータが数独である
    //! @retval false 認識したデータが数独でない
    bool recognize_number(void);

    //! @brief 画像を二値化する
    //! @param target_frame   処理対象画像
    //! @param threshold_type 反転する場合:THRESH_BINARY_INV しない場合:THRESH_BINARY
    void make_binary_frame(Mat &target_frame, int threshold_type);

    //! @brief  画像から数独の輪郭を抽出して直線近似する
    //! @retval true  輪郭を抽出できた
    //! @retval false 輪郭をできなかった
    bool get_outer_contour(void);

    //! @brief  ホモグラフィー行列を取得する
    //! @param  contour    変換元座標平面の４点
    //! @param  bound_rect 変換先座標平面の４点
    //! @return 取得したホモグラフィー行列
    Mat get_homography(vector<Point> &contour, Rect &bound_rect);

    //! @brief 画像から枠線を消す
    void delete_grid(void);

    //! @brief 画像を回転する
    //! @param phase 回転角度 [0:0 1:90 2:180 3:270]
    void rotate_frame(int phase = 1);

    //! @brief  数独を解く
    //! @retval true  数独を解けた
    //! @retval false 数独を解けなかった
    bool sudoku_solve(void);

    //! @brief  数独の問題として適切かどうかの判定
    //! @param  count 初期値の数
    //! @retval true  適切
    //! @retval false 不適切
    bool is_sudoku(int count);
};
