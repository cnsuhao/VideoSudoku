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

using namespace cv;
using namespace std;

VideoSudoku::VideoSudoku(void):
    cells_number(9),
    result_min_size(400),
    input_name("Input frame"),
    temp_name("Temp frame"),
    result_name("Result frame"),
    ocr_type("SVMOCR"),
    model("model/normalized30x30.model"),
    contour_line_color(0, 255, 0),
    frame_background_color(255, 255, 255),
    initial_text_color(255, 0, 0),
    result_text_color(0, 0, 255),
    cell_line_color(0, 255, 0),
    result_size(0),
    cell_size(0),
    text_offset(0),
    initialized(false),
    ocr(nullptr),
    capture(),
    input_frame(),
    temp_frame(),
    result_frame(),
    contour()
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
        finalize();

        return 1;
    }

    ocr = sudokuOCRFactory(ocr_type);

    if(ocr == nullptr)
    {
        finalize();

        return 2;
    }

    if(!ocr->initialize(model))
    {
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
#ifdef VS_DEBUG
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
#ifdef VS_DEBUG
    destroyWindow(temp_name);
#endif
}

bool VideoSudoku::capture_video(void)
{
    if(!initialized)
    {
        return false;
    }

    if(!capture.read(input_frame))
    {
        return false;
    }

    return true;
}

void VideoSudoku::display(bool results_availability)
{
    if(!initialized)
    {
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
#ifdef VS_DEBUG
    imshow(temp_name, temp_frame);
#endif
}

bool VideoSudoku::fix_outer_frame(void)
{
    if(!initialized)
    {
        return false;
    }

    input_frame.copyTo(temp_frame);
    make_binary_frame(temp_frame, THRESH_BINARY_INV);

    if(!get_outer_contour())
    {
        return false;
    }

    Mat contour_matrix(contour);
    double epsilon = 0.01 * arcLength(contour_matrix, true);
    approxPolyDP(contour_matrix, contour, epsilon, true);

    if(is_sudoku_contour())
    {
        input_frame.copyTo(temp_frame);

        Rect bound_rect = boundingRect(contour);
        Mat homography = get_homography(contour, bound_rect);

        warpPerspective(temp_frame, temp_frame, homography, temp_frame.size());

        temp_frame = Mat(temp_frame, bound_rect);
        resize(temp_frame, temp_frame, Size(result_size, result_size));

        make_binary_frame(temp_frame, THRESH_BINARY);

        return true;
    }

    return false;
}

bool VideoSudoku::solve(void)
{
    if(!initialized)
    {
        return false;
    }

    delete_grid();

    if(recognize_number())
    {
        if(sudoku_solve())
        {
            return true;
        }
    }

    return false;
}

bool VideoSudoku::recognize_number(void)
{
    int all_cells_number = cells_number * cells_number;

    // 数独の初期値として適切かどうか調べるために、文字を認識する前にすべてのマスに数字が詰まっているとみなす。
    // 数字の詰まっているマスの数を保持しておき、数字の無いマスを見つけ次第差し引いていく。
    // 数独の初期値として適切でないと判断した時点で文字認識を止める。
    int count = all_cells_number;
    int number = 0;

    Point position;
    Rect cut_area(0, 0, cell_size, cell_size);

    for(int i = 0; i < all_cells_number; i++)
    {
        position.x = (i % cells_number) * cell_size;
        position.y = (i / cells_number) * cell_size;

        cut_area.x = position.x;
        cut_area.y = position.y;

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

    input_problem[all_cells_number + 1] = '\0';

    return true;
}

void VideoSudoku::frame_initialize(Mat &frame, int size)
{
    frame = Mat(Size(size, size), CV_8UC3, frame_background_color);
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
    int all_cells_number = cells_number * cells_number;

    Point position;

    for(int i = 0; i < all_cells_number; i++)
    {
        position.x = ((i % cells_number) * cell_size) + text_offset;
        position.y = ((1 + (i / cells_number)) * cell_size) - text_offset;

        char input = input_problem[i];
        char result = result_problem[i];

        // 結果には初期値も含まれているため、初期値は後で上から描画する。
        if((result >= '1') && (result <= '9'))
        {
            string text(1, result);
            putText(result_frame, text, position, FONT_HERSHEY_SIMPLEX, 1, result_text_color, 3);
        }

        if((input >= '1') && (input <= '9'))
        {
            string text(1, input);
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
    }

    for(int i = 0; i < cells_number; i++)
    {
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

    vector<vector<Point>> contours;
    findContours(temp_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    if(contours.empty())
    {
        return false;
    }

    int size = static_cast<int>(contours.size());

    for(int i = 0; i < size; i++)
    {
        long area = contourArea(contours.at(i));

        if(max_area < area)
        {
            max_area = area;
            max_area_contour = i;
        }
    }

    contour = contours.at(max_area_contour);

    return true;
}

Mat VideoSudoku::get_homography(vector<Point> &frame_contour, Rect &bound_rect)
{
    int x = bound_rect.x;
    int y = bound_rect.y;
    int width = bound_rect.width;
    int height = bound_rect.height;

    vector<Point> corner_contour;

    // 矩形と輪郭の頂点を対応させる。
    // 輪郭の頂点0と頂点2のx座標の関係によって、矩形の頂点指定順を変更する。
    if(contour.at(0).x < contour.at(2).x)
    {
        corner_contour.push_back(Point(x, y));
        corner_contour.push_back(Point(x, y + height));
        corner_contour.push_back(Point(x + width, y + height));
        corner_contour.push_back(Point(x + width, y));
    }
    else
    {
        corner_contour.push_back(Point(x + width, y));
        corner_contour.push_back(Point(x, y));
        corner_contour.push_back(Point(x, y + height));
        corner_contour.push_back(Point(x + width, y + height));
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
    }

    for(int i = 0; i <= cells_number; i++)
    {
        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(temp_frame, pt1, pt2, delete_line_color, line_thickness);
    }
}

bool VideoSudoku::sudoku_solve(void)
{
    bool result_code = static_cast<bool>(solve_dlx_sudoku(input_problem, result_problem));

#ifdef VS_DEBUG
    DEBUG(" input  : %s", input_problem);
    DEBUG(" result : %s", result_problem);
    DEBUG(" code   : %d", result_code);
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
