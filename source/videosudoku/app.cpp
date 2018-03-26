namespace
{
/*const auto contour_line_color = Scalar(0, 255, 0);         //!< 輪郭線色
const auto frame_background_color = Scalar(255, 255, 255); //!< 画像の背景色
const auto initial_text_color = Scalar(255, 0, 0);         //!< 数独初期値の文字色
const auto result_text_color = Scalar(0, 0, 255);          //!< 数独を解いた結果の文字色
const auto cell_line_color = Scalar(0, 255, 0);            //!< 数独を解いた結果の枠線色*/
}

namespace videosudoku
{
/*int VideoSudoku::initialize(const int size, const int device_id)
{
    capture.open(device_id);

    if(!capture.isOpened()) return 1;

    result_size = size < result_min_size ? result_min_size : size;
    cell_size = result_size / cells_number;
    text_offset = (cell_size - getTextSize("0", FONT_HERSHEY_SIMPLEX, 1, 3, 0).width) / 2;

    frame_initialize(input_frame, result_size);
    frame_initialize(temp_frame, result_size);
    frame_initialize(result_frame, result_size);
}

void VideoSudoku::display(const bool results_availability)
{
    if(is_sudoku_contour())
    {
        polylines(input_frame, contour, true, contour_line_color, 2);
    }

    if(results_availability)
    {
        frame_initialize(result_frame, result_size);
        draw_result();
    }

    imshow(input_name, input_frame);
    imshow(result_name, result_frame);
}

cv::Mat fix_outer_frame()
{
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

void VideoSudoku::draw_result()
{
    Point position;

    for(auto i = 0; i < all_cells_number; ++i)
    {
        position.x = ((i % cells_number) * cell_size) + text_offset;
        position.y = ((1 + (i / cells_number)) * cell_size) - text_offset;

        const auto input = input_problem[i];
        const auto result = result_problem[i];

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

Mat VideoSudoku::get_homography(const vector<Point> &frame_contour, const Rect &bound_rect) const
{
    const auto x = bound_rect.x;
    const auto y = bound_rect.y;
    const auto width = bound_rect.width;
    const auto height = bound_rect.height;

    vector<Point> corner_contour;

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

    for(auto i = 0; i <= cells_number; ++i)
    {
        pt1.x = cell_size * i;
        pt1.y = 0;
        pt2.x = cell_size * i;
        pt2.y = result_size;

        line(temp_frame, pt1, pt2, frame_background_color, line_thickness);
    }

    for(auto i = 0; i <= cells_number; ++i)
    {
        pt1.x = 0;
        pt1.y = cell_size * i;
        pt2.x = result_size;
        pt2.y = cell_size * i;

        line(temp_frame, pt1, pt2, frame_background_color, line_thickness);
    }
}*/
}
