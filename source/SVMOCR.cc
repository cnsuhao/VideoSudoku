//!
//! @file  SVMOCR.cc
//! @brief SVMOCR クラス実装
//!

#include "SVMOCR.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace
{
using namespace cv;
using namespace std;

constexpr auto DEFAULT_MODEL_FILE = "resource/model/normalized30x30.model";
}

namespace videosudoku
{
bool SVMOCR::initialize(const char *initialize_file_name)
{
    if(initialize_file_name)
    {
        model = svm_load_model(initialize_file_name);
    }
    else
    {
        model = svm_load_model(DEFAULT_MODEL_FILE);
    }

    if(!model) return false;

    // probability にアクセスするため、ラベルに対応する index のテーブルを作成しておく。
    for(auto i = 0; i < NR_CLASS; ++i)
    {
        for(auto j = 0; j < NR_CLASS; ++j)
        {
            if(model->label[j] == i)
            {
                label_to_index[i] = j;
            }
        }
    }

    return true;
}

void SVMOCR::finalize(void)
{
    svm_free_and_destroy_model(&model);
}

int SVMOCR::recognize_number(Mat &mat)
{
    compute_feature(mat, data);

    return predict(data);
}

void SVMOCR::compute_feature(Mat &mat, unsigned char *data) const
{
    normalize(mat, mat);

    resize(mat, mat, Size(IMAGE_RC, IMAGE_RC));

    // 画像データをそのまま１次元の判定データにする。
    for(auto row = 0; row < mat.rows; ++row)
    {
        auto ptr = mat.ptr<unsigned char>(row);

        for(auto col = 0; col < mat.cols; ++col)
        {
            data[row * mat.cols + col] = *ptr++;
        }
    }
}

void SVMOCR::normalize(const Mat &src, Mat &dst) const
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Rect rect;

    auto area = 0.0;

    findContours(src.clone(), contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    auto max_area = 0.0;
    auto max_area_index = -1l;

    for(auto i = 0u; i < contours.size(); ++i)
    {
        rect = boundingRect(contours[i]);

        if(rect.x < src.cols * 5 / 100 || rect.y < src.rows * 5 / 100)
        {
            continue;
        }

        if(rect.width < src.cols / 10 || rect.height < src.rows * 4 / 10)
        {
            continue;
        }

        area = contourArea(contours[i]);

        if(area > max_area)
        {
            max_area = area;
            max_area_index = static_cast<long>(i);
        }
    }

    if(max_area_index >= 0)
    {
        rect = boundingRect(contours[static_cast<unsigned long>(max_area_index)]);

        auto x = rect.x + (rect.width / 2) - (rect.height / 2);

        if(x < 0)
        {
            x = 0;
        }

        const auto w = x + rect.height > src.cols ? src.cols - x : rect.height;

        dst = src({x, rect.y, w, rect.height});
    }
    else
    {
        dst = src({src.cols * 5 / 100, src.rows * 5 / 100, src.cols * 9 / 10, src.rows * 9 / 10});
    }
}

int SVMOCR::predict(unsigned char *data)
{
    // svm_predict*() を利用するためにデータの変換を行う。
    for(auto i = 0; i < DATA_SIZE; ++i)
    {
        x[i].index = i + 1;
        x[i].value = data[i];
    }

    x[DATA_SIZE].index = -1;

    // probability が不要であれば、svm_predict() でもよい。
    return static_cast<int>(svm_predict_probability(model, x, probability));
}
}
