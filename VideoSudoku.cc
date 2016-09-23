//!
//! @file      VideoSudoku.cc
//! @brief     VideoSudoku クラス実装
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#include "VideoSudoku.h"
#include "dlx_sudoku.h"

#define VIDEOSUDOKU_DEBUG 0

using namespace cv;
using namespace std;

VideoSudoku::VideoSudoku(void):
    cells_number(9),
    result_min_size(400),
    input_name("Input frame"),
    temp_name("Temp frame"),
    result_name("Result frame"),
    ocr_type("SVMOCR"),
    model("normalized30x30.model"),
    contour_line_color(0, 255, 0),
    frame_background_color(255, 255, 255),
    initial_text_color(255, 0, 0),
    result_text_color(0, 0, 255),
    cell_line_color(0, 255, 0),
    rotate_phase(0),
    initialized(false),
    ocr(nullptr)
{
    input_problem = new char[(cells_number * cells_number) + 1];
    result_problem = new char[(cells_number * cells_number) + 1];
}

VideoSudoku::~VideoSudoku(void)
{
    finalize();

    delete[] input_problem;
    delete[] result_problem;
}

int VideoSudoku::initialize(int size, int device_id)
{
    finalize();

    capture.open(device_id);

    if(!capture.isOpened())
    {
        ERROR("カメラデバイス(id:%d)を開けませんでした。\n", device_id);
        finalize();

        return 1;
    }

    ocr = sudokuOCRFactory(ocr_type);

    if(ocr == nullptr)
    {
        ERROR("文字認識オブジェクトを初期化できませんでした。\n");
        finalize();

        return 2;
    }

    if(!ocr->initialize(model))
    {
        ERROR("モデルデータ(%s)を読み込めませんでした。\n", model);
        finalize();

        return 3;
    }

    if(size < result_min_size)
    {
        size = result_min_size;
    }

    result_size = size;
    cell_size = result_size / cells_number;
    text_offset = (cell_size - getTextSize("0", FONT_HERSHEY_SIMPLEX, 1, 3, 0).width) / 2;

    frame_initialize(input_frame, result_min_size);
    frame_initialize(temp_frame, result_min_size);
    frame_initialize(result_frame, result_size);

    namedWindow(input_name, CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);
    namedWindow(result_name, CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);
#if VIDEOSUDOKU_DEBUG
    namedWindow(temp_name, CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);
#endif

    initialized = true;

    return 0;
}

void VideoSudoku::finalize(void)
{
    if(ocr != nullptr)
    {
        ocr->finalize();

        delete ocr;
        ocr = nullptr;
    }

    if(capture.isOpened())
    {
        capture.release();
    }

    initialized = false;

    destroyWindow(input_name);
    destroyWindow(result_name);
#if VIDEOSUDOKU_DEBUG
    destroyWindow(temp_name);
#endif
}

bool VideoSudoku::capture_video(void)
{
    if(!initialized)
    {
        ERROR("オブジェクトが正しく初期化されていません。\n");

        return false;
    }

    if(!capture.read(input_frame))
    {
        ERROR("カメラ入力を読み込めませんでした。\n");

        return false;
    }

    return true;
}

void VideoSudoku::display(bool results_availability)
{
    if(!initialized)
    {
        ERROR("オブジェクトが正しく初期化されていません。\n");

        return;
    }

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
#if VIDEOSUDOKU_DEBUG
    imshow(temp_name, temp_frame);
#endif
}

bool VideoSudoku::fix_outer_frame(void)
{
    if(!initialized)
    {
        ERROR("オブジェクトが正しく初期化されていません。\n");

        return false;
    }

    input_frame.clone().copyTo(temp_frame);
    make_binary_frame(temp_frame, THRESH_BINARY_INV);

    if(!get_outer_contour())
    {
        return false;
    }

    // 近似精度を輪郭の外周の1/100にすることで精度よく近似できた
    double epsilon = 0.01 * arcLength(Mat(contour), true);
    approxPolyDP(Mat(contour), contour, epsilon, true);

    if(is_sudoku_contour())
    {
        Mat local_temp_frame = input_frame.clone();

        Rect bound_rect = boundingRect(contour);

        Mat homography = get_homography(contour, bound_rect);

        warpPerspective(local_temp_frame, local_temp_frame, homography, local_temp_frame.size());

        local_temp_frame = Mat(local_temp_frame, bound_rect);
        resize(local_temp_frame, local_temp_frame, Size(result_size, result_size));

        local_temp_frame.clone().copyTo(temp_frame);
        make_binary_frame(temp_frame, THRESH_BINARY);

        return true;
    }

    return false;
}

bool VideoSudoku::solve(void)
{
    if(!initialized)
    {
        ERROR("オブジェクトが正しく初期化されていません。\n");

        return false;
    }

    delete_grid();

    // 数独の向きによらずに結果を得るため、数独を解けなかったときに画像を回転させて再試行するようにした
    // また、前回の回転状況を保持することで無駄な回転を抑えた
    rotate_frame(rotate_phase);

    for(int i = 0; i <= 3; i++)
    {
        if(recognize_number())
        {
            if(sudoku_solve())
            {
                return true;
            }
        }

        rotate_frame();

        rotate_phase++;
        rotate_phase %= 4;
    }

    return false;
}

bool VideoSudoku::recognize_number(void)
{
    // 数独の初期値として適切かどうか調べるために、文字を認識する前にすべてのマスに数字が詰まっているとみなす
    // 数字の詰まっているマスの数を保持しておき、数字の無いマスを見つけ次第差し引いていく
    // 数独の初期値として適切でないと判断した時点で文字認識を止める
    int count = cells_number * cells_number;
    int number = 0;
    Point position;

    for(int i = 0; i < (cells_number * cells_number); i++)
    {
        position.x = (i % cells_number) * cell_size;
        position.y = (i / cells_number) * cell_size;

        Rect cut_area(position.x, position.y, cell_size, cell_size);
        Mat cut_img(temp_frame, cut_area);

        number = ocr->recognize_number(cut_img);

        if(number == 0)
        {
            count--;
        }

        if(!is_sudoku(count))
        {
            return false;
        }

        input_problem[i] = static_cast<char>(number) + '0';
    }

    input_problem[(cells_number * cells_number) + 1] = '\0';

    return true;
}

void VideoSudoku::frame_initialize(Mat &frame, int size)
{
    Mat back_ground(Size(size, size), CV_8UC3, frame_background_color);
    back_ground.clone().copyTo(frame);
}

bool VideoSudoku::is_sudoku_contour(void)
{
    if(contour.size() != 4)
    {
        return false;
    }

    if(!isContourConvex(contour))
    {
        return false;
    }

    long contour_area = contourArea(contour);
    long temp_frame_area = temp_frame.rows * temp_frame.cols;

    if(contour_area >= (temp_frame_area / 2))
    {
        return false;
    }

    return true;
}

void VideoSudoku::draw_result(void)
{
    string text;
    Point position;

    for(int i = 0; i < (cells_number * cells_number); i++)
    {
        position.x = ((i % cells_number) * cell_size) + text_offset;
        position.y = ((1 + (i / cells_number)) * cell_size) - text_offset;

        if((result_problem[i] >= '1') && (result_problem[i] <= '9'))
        {
            text = string(1, result_problem[i]);
            putText(result_frame, text, position, FONT_HERSHEY_SIMPLEX, 1, result_text_color, 3);
        }

        if((input_problem[i] >= '1') && (input_problem[i] <= '9'))
        {
            text = string(1, input_problem[i]);
            putText(result_frame, text, position, FONT_HERSHEY_SIMPLEX, 1, initial_text_color, 3);
        }
    }
}

void VideoSudoku::draw_cell(void)
{
    Point pt1, pt2;

    for(int i = 0; i < cells_number; i++)
    {
        pt1.x = cell_size * i;
        pt1.y = 0;
        pt2.x = cell_size * i;
        pt2.y = result_size;

        line(result_frame, pt1, pt2, cell_line_color, 1);

        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(result_frame, pt1, pt2, cell_line_color, 1);
    }
}

void VideoSudoku::make_binary_frame(Mat &target_frame, int threshold_type)
{
    cvtColor(target_frame, target_frame, COLOR_RGB2GRAY);

    adaptiveThreshold(target_frame, target_frame, 255.0, ADAPTIVE_THRESH_MEAN_C, threshold_type, 23, 5.5);
}

bool VideoSudoku::get_outer_contour(void)
{
    int max_area_contour = 0;
    long max_area = 0;
    long temp_area;

    vector<vector<Point>> contours;

    Mat local_temp_frame = temp_frame.clone();
    findContours(local_temp_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    if(contours.empty())
    {
        return false;
    }

    for(int i = 0; i < static_cast<int>(contours.size()); i++)
    {
        temp_area = contourArea(contours.at(i));

        if(max_area < temp_area)
        {
            max_area = temp_area;
            max_area_contour = i;
        }
    }

    contour = contours.at(max_area_contour);

    return true;
}

Mat VideoSudoku::get_homography(vector<Point> &frame_contour, Rect &bound_rect)
{
    vector<Point> corner_contour;

    // 輪郭の頂点1と頂点3のx座標の関係によって、矩形の頂点指定順を変更する
    if(contour.at(0).x < contour.at(2).x)
    {
        corner_contour.push_back(Point(bound_rect.x, bound_rect.y));
        corner_contour.push_back(Point(bound_rect.x, bound_rect.y + bound_rect.height));
        corner_contour.push_back(Point(bound_rect.x + bound_rect.width, bound_rect.y + bound_rect.height));
        corner_contour.push_back(Point(bound_rect.x + bound_rect.width, bound_rect.y));
    }
    else
    {
        corner_contour.push_back(Point(bound_rect.x + bound_rect.width, bound_rect.y));
        corner_contour.push_back(Point(bound_rect.x, bound_rect.y));
        corner_contour.push_back(Point(bound_rect.x, bound_rect.y + bound_rect.height));
        corner_contour.push_back(Point(bound_rect.x + bound_rect.width, bound_rect.y + bound_rect.height));
    }

    return findHomography(frame_contour, corner_contour);
}

void VideoSudoku::delete_grid(void)
{
    int line_thickness = (12 * result_size) / 500;
    Scalar delete_line_color(255, 255, 255);

    Point pt1, pt2;

    for(int i = 0; i <= cells_number; i++)
    {
        pt1.x = cell_size * i;
        pt1.y = 0;
        pt2.x = cell_size * i;
        pt2.y = result_size;

        line(temp_frame, pt1, pt2, delete_line_color, line_thickness);

        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(temp_frame, pt1, pt2, delete_line_color, line_thickness);
    }
}

void VideoSudoku::rotate_frame(int phase)
{
    if(phase == 0)
    {
        return;
    }

    Point frame_center(result_size / 2, result_size / 2);

    Mat rotation_matrix = getRotationMatrix2D(frame_center, phase * 90, 1);
    warpAffine(temp_frame, temp_frame, rotation_matrix, Size(result_size, result_size));
}

bool VideoSudoku::sudoku_solve(void)
{
    bool result_code = static_cast<bool>(solve_dlx_sudoku(input_problem, result_problem));

#if VIDEOSUDOKU_DEBUG
    DEBUG("input:  %s", input_problem);
    DEBUG("result: %s", result_problem);
    DEBUG("code:   %d", result_code);
#endif

    return result_code;
}

bool VideoSudoku::is_sudoku(int count)
{
    if(count < 17)
    {
        return false;
    }

    return true;
}
