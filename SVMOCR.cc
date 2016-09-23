//!
//! @file      SVMOCR.cc
//! @brief     SVMOCR クラス実装
//!

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/types.h>
#include <dirent.h>

#include "SVMOCR.h"

#define DEFAULT_MODEL_FILE "normalized30x30.model"

using namespace std;

bool SVMOCR::initialize(const char *initialize_file_name)
{
    // libsvm のモデルのロード (モデルは別途、学習済み)
    if(initialize_file_name != nullptr)
    {
        model = svm_load_model(initialize_file_name);
    }
    else
    {
        model = svm_load_model(DEFAULT_MODEL_FILE);
    }

    if(model != nullptr)
    {
        // probability にアクセスするため、ラベルに対応する index のテーブルを作成しておく　
        for(int i = 0; i < 10; i++)
        {
            for(int j = 0; j < 10; j++)
            {
                if(model->label[j] == i)
                {
                    label_to_index[i] = j;
                }
            }
        }

        return true;
    }

    return false;
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


void SVMOCR::print_image(Mat &mat)
{
    // デバッグ用のデータダンプ
    for(int row = 0; row < mat.rows; row++)
    {
        const unsigned char *ptr = mat.ptr<unsigned char>(row);

        for(int col = 0; col < mat.cols; col++)
        {
            printf("%02x", *ptr++);
        }

        putchar('\n');
    }
}

void SVMOCR::compute_feature(Mat &mat, unsigned char *data)
{
    normalize(mat, mat);

    // サイズを所定のサイズ (IMAGE_RC x IMAGE_RC) にする
    resize(mat, mat, Size(IMAGE_RC, IMAGE_RC));

    // 画像データをそのまま１次元の判定データにする
    for(int row = 0; row < mat.rows; row++)
    {
        const unsigned char *ptr = mat.ptr<unsigned char>(row);

        for(int col = 0; col < mat.cols; col++)
        {
            data[row * mat.cols + col] = *ptr++;
        }
    }
}

void SVMOCR::normalize(Mat &src, Mat &dst)
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Rect rect;
    double area;

    findContours(src.clone(), contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    double max_area = 0.0;
    int max_area_index = -1;

    for(int i = 0; i < static_cast<int>(contours.size()); i++)
    {
        rect = boundingRect(contours[i]);

        if((rect.x < (src.cols * 5 / 100)) || (rect.y < (src.rows * 5 / 100)))
        {
            continue;
        }

        if((rect.width < (src.cols / 10)) || (rect.height < (src.rows * 4 / 10)))
        {
            continue;
        }

        area = contourArea(contours[i]);

        if(area > max_area)
        {
            max_area = area;
            max_area_index = i;
        }
    }

    if(max_area_index >= 0)
    {
        rect = boundingRect(contours[max_area_index]);

        int x = rect.x + (rect.width / 2) - (rect.height / 2);
        x = x < 0 ? 0 : x;

        int w = (x + rect.height) > src.cols ? src.cols - x : rect.height;

        dst = src(Rect(x, rect.y, w, rect.height)); 
    }
    else
    {
        dst = src(Rect(src.cols * 5 / 100, src.rows * 5 / 100, src.cols * 9 / 10, src.rows * 9 / 10));
    }
}

int SVMOCR::predict(unsigned char *data)
{
    // svm_predict*() を利用するためにデータの変換を行う
    for(int i = 0; i < DATA_SIZE; i++)
    {
        x[i].index = i + 1;
        x[i].value = data[i];
    }

    x[DATA_SIZE].index = -1;

    // svm_predict*() を呼び出す
    // probability が不要であれば、svm_predict() でもよい。
    double d = svm_predict_probability(model, x, probability);

    return static_cast<int>(d);
}

bool SVMOCR::test_file(const char *file_name, int label)
{
    Mat mat = imread(file_name, 0);

    if(mat.data != nullptr)
    {
        medianBlur(mat, mat, 5);
        adaptiveThreshold(mat, mat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 25, 10);

        int val = recognize_number(mat);

        if(val == label)
        {
            return true;
        }
        else
        {
            printf("\n Fail val = %d, label = %d\n", val, label);

            for(int i = 0; i < 10; i++)
            {
                printf("\t%d : %f\n", i, probability[label_to_index[i]]);
            }

            return false;
        }
    }
    else
    {
        printf(" mat.data = %p\n", mat.data);
    }

    return false;
}

void SVMOCR::test_dir(const char *dir_name)
{
    DIR* dir = opendir(dir_name);
    int ok, ng;

    if(dir != nullptr)
    {
        struct dirent* entry;

        ok = ng = 0;

        while((entry = readdir(dir)) != nullptr)
        {
            char *pos;

            if(((pos = strstr(entry->d_name, ".jpg")) != nullptr) && (strlen(pos) == 4))
            {
                string s(dir_name);

                s += entry->d_name;

                if(test_file(s.c_str(), static_cast<int>(*entry->d_name - '0')))
                {
                    ok++;
                }
                else
                {
                    ng++;
                }
            }
        }

        printf(" result = %f%% (%d / %d)\n", static_cast<double>(ok) / (ok + ng) * 100.0, ok, ok + ng);

        closedir(dir);
    }
}
