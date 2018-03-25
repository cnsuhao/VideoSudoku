//!
//! @file      VideoSudoku.cc
//! @brief     VideoSudoku クラス実装
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#include "VideoSudoku.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "dlx_sudoku.h"

namespace
{
using namespace cv;
using namespace std;

constexpr auto cells_number = 9; //!< 一辺のマスの数

constexpr auto all_cells_number = cells_number * cells_number; //!< 全てのマスの数

constexpr auto result_min_size = 400;  //!< 結果画像の一辺の長さの最小値

constexpr auto input_name = "Input";   //!< 入力画像ウィンドウの名前
constexpr auto result_name = "Result"; //!< 結果画像ウィンドウの名前

#ifdef VIDEOSUDOKU_DEBUG
constexpr auto temp_name = "Temp"; //!< 作業用画像ウィンドウの名前
#endif

constexpr auto ocr_type = "SVMOCR"; //!< 文字認識オブジェクトの種類

constexpr auto model = "resource/model/normalized30x30.model"; //!< 文字認識に使うモデルデータのパス

const auto contour_line_color = Scalar(0, 255, 0);         //!< 輪郭線色
const auto frame_background_color = Scalar(255, 255, 255); //!< 画像の背景色
const auto initial_text_color = Scalar(255, 0, 0);         //!< 数独初期値の文字色
const auto result_text_color = Scalar(0, 0, 255);          //!< 数独を解いた結果の文字色
const auto cell_line_color = Scalar(0, 255, 0);            //!< 数独を解いた結果の枠線色
}

namespace videosudoku
{
VideoSudoku::VideoSudoku()
{
    input_problem = new char[all_cells_number + 1];
    result_problem = new char[all_cells_number + 1];
}

VideoSudoku::~VideoSudoku()
{
    finalize();

    delete[] input_problem;
    delete[] result_problem;
}

int VideoSudoku::initialize(const int size, const int device_id)
{
    finalize();

    capture.open(device_id);

    if(!capture.isOpened()) return 1;

    ocr = sudokuOCRFactory(ocr_type);

    if(!ocr) return 2;

    if(!ocr->initialize(model)) return 3;

    result_size = size < result_min_size ? result_min_size : size;
    cell_size = result_size / cells_number;
    text_offset = (cell_size - getTextSize("0", FONT_HERSHEY_SIMPLEX, 1, 3, 0).width) / 2;

    frame_initialize(input_frame, result_size);
    frame_initialize(temp_frame, result_size);
    frame_initialize(result_frame, result_size);

    initialized = true;

    return 0;
}

void VideoSudoku::finalize()
{
    if(ocr)
    {
        ocr->finalize();

        delete ocr;
        ocr = nullptr;
    }

    capture.release();

    initialized = false;
}

bool VideoSudoku::capture_video()
{
    if(!initialized || !capture.isOpened()) return false;

    capture >> input_frame;

    return true;
}

void VideoSudoku::display(const bool results_availability)
{
    if(!initialized) return;

    if(is_sudoku_contour())
    {
        polylines(input_frame, contour, true, contour_line_color, 2);
    }

    if(results_availability)
    {
        frame_initialize(result_frame, result_size);
        draw_result();
        draw_cell();
    }

    imshow(input_name, input_frame);
    imshow(result_name, result_frame);

#ifdef VIDEOSUDOKU_DEBUG
    imshow(temp_name, temp_frame);
#endif
}

bool VideoSudoku::solve()
{
    if(!initialized) return false;

    if(!fix_outer_frame()) return false;

    delete_grid();

    if(recognize_number())
    {
        if(sudoku_solve()) return true;
    }

    return false;
}

bool VideoSudoku::fix_outer_frame()
{
    input_frame.copyTo(temp_frame);
    make_binary_frame(temp_frame, THRESH_BINARY_INV);

    input_frame.copyTo(temp_frame);

    const auto bound_rect = boundingRect(contour);
    const auto homography = get_homography(contour, bound_rect);

    warpPerspective(temp_frame, temp_frame, homography, temp_frame.size());

    temp_frame = {temp_frame, bound_rect};
    resize(temp_frame, temp_frame, {result_size, result_size});

    make_binary_frame(temp_frame, THRESH_BINARY);

    return true;
}

bool VideoSudoku::recognize_number()
{
    // 数独の初期値として適切かどうか調べるために、文字を認識する前にすべてのマスに数字が詰まっているとみなす。
    // 数字の詰まっているマスの数を保持しておき、数字の無いマスを見つけ次第差し引いていく。
    // 数独の初期値として適切でないと判断した時点で文字認識を止める。
    auto count = all_cells_number;
    auto number = 0;

    Point position;

    for(auto i = 0; i < all_cells_number; ++i)
    {
        position.x = (i % cells_number) * cell_size;
        position.y = (i / cells_number) * cell_size;

        const Rect cut_area = {position.x, position.y, cell_size, cell_size};

        Mat cut_frame = {temp_frame, cut_area};

        number = ocr->recognize_number(cut_frame);

        if(number == 0)
        {
            count--;
        }

        if(count < 17) return false;

        input_problem[i] = static_cast<char>(number) + '0';
    }

    input_problem[all_cells_number + 1] = '\0';

    return true;
}

void VideoSudoku::frame_initialize(Mat &frame, const int size) const
{
    frame = Mat(Size(size, size), CV_8UC3, frame_background_color);
}

void VideoSudoku::draw_result()
{
    Point position;

    for(auto i = 0; i < all_cells_number; ++i)
    {
        position.x = ((i % cells_number) * cell_size) + text_offset;
        position.y = ((1 + (i / cells_number)) * cell_size) - text_offset;

        const auto input = input_problem[i];
        const auto result = result_problem[i];

        // 結果には初期値も含まれているため、初期値は結果の上から描画する。
        if(result >= '1' && result <= '9')
        {
            putText(result_frame, {1, result}, position, FONT_HERSHEY_SIMPLEX, 1, result_text_color, 3);
        }

        if(input >= '1' && input <= '9')
        {
            putText(result_frame, {1, input}, position, FONT_HERSHEY_SIMPLEX, 1, initial_text_color, 3);
        }
    }
}

void VideoSudoku::draw_cell()
{
    Point pt1, pt2;

    // 縦線を描く。
    for(auto i = 0; i < cells_number; ++i)
    {
        pt1.x = cell_size * i;
        pt1.y = 0;
        pt2.x = cell_size * i;
        pt2.y = result_size;

        line(result_frame, pt1, pt2, cell_line_color, 1);
    }

    // 横線を描く。
    for(auto i = 0; i < cells_number; ++i)
    {
        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(result_frame, pt1, pt2, cell_line_color, 1);
    }
}

Mat VideoSudoku::get_homography(const vector<Point> &frame_contour, const Rect &bound_rect) const
{
    const auto x = bound_rect.x;
    const auto y = bound_rect.y;
    const auto width = bound_rect.width;
    const auto height = bound_rect.height;

    vector<Point> corner_contour;

    // 矩形と輪郭の頂点を対応させる。
    // 輪郭の頂点0と頂点2のx座標の関係によって、矩形の頂点指定順を変更する。
    if(contour[0].x < contour[2].x)
    {
        corner_contour.push_back({x, y});
        corner_contour.push_back({x, y + height});
        corner_contour.push_back({x + width, y + height});
        corner_contour.push_back({x + width, y});
    }
    else
    {
        corner_contour.push_back({x + width, y});
        corner_contour.push_back({x, y});
        corner_contour.push_back({x, y + height});
        corner_contour.push_back({x + width, y + height});
    }

    return findHomography(frame_contour, corner_contour);
}

void VideoSudoku::delete_grid()
{
    const auto line_thickness = (12 * result_size) / 500;

    Point pt1, pt2;

    // 縦線を削る。
    for(auto i = 0; i <= cells_number; ++i)
    {
        pt1.x = cell_size * i;
        pt1.y = 0;
        pt2.x = cell_size * i;
        pt2.y = result_size;

        line(temp_frame, pt1, pt2, frame_background_color, line_thickness);
    }

    // 横線を削る。
    for(auto i = 0; i <= cells_number; ++i)
    {
        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(temp_frame, pt1, pt2, frame_background_color, line_thickness);
    }
}

bool VideoSudoku::sudoku_solve()
{
    const auto result_code = solve_dlx_sudoku(input_problem, result_problem);

#ifdef VIDEOSUDOKU_DEBUG
    DEBUG(" input  : %s", input_problem);
    DEBUG(" result : %s", result_problem);
    DEBUG(" code   : %d", result_code);
#endif

    return result_code == 1;
}
}
