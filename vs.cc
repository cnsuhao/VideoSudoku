//!
//! @file      vs.cc
//! @brief     メインエントリ 実装
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#include <stdlib.h>

#include "VideoSudoku.h"
#include "debuglog.h"

#define WINDOW_SIZE 400 //!< ウィンドウサイズ
#define CAMERA_ID 0     //!< カメラデバイスのID
#define WAIT_TIME 60    //!< キー入力の待ち時間
#define CODE_ESCAPE 27  //!< Escapeキーのキーコード
#define CODE_SPACE 32   //!< Spaceキーのキーコード

#define CAMERA_OPENING_ERROR "The camera device wasn't able to be opened.\n" //!< カメラデバイスを開けなかったエラーメッセージ
#define CAMERA_READING_ERROR "The camera device wasn't able to be read.\n"   //!< カメラデバイスを読めなかったエラーメッセージ
#define OCR_INITIALIZATION_ERROR "The OCR initialization was failed.\n"      //!< OCRを初期化できなかったエラーメッセージ
#define MODEL_OPENING_ERROR "The model file wasn't able to be opened.\n"     //!< モデルファイルを開けなかったエラーメッセージ

int main(int argc, char **argv)
{
    VideoSudoku vs;

    bool state_holding = false;
    bool results_availability = false;

    switch(vs.initialize(WINDOW_SIZE, CAMERA_ID))
    {
        case 1:
            ERROR(CAMERA_OPENING_ERROR);

            return EXIT_FAILURE;

        case 2:
            ERROR(OCR_INITIALIZATION_ERROR);

            return EXIT_FAILURE;

        case 3:
            ERROR(MODEL_OPENING_ERROR);

            return EXIT_FAILURE;
    }

    while(true)
    {
        if(!vs.capture_video())
        {
            ERROR(CAMERA_READING_ERROR);

            return EXIT_FAILURE;
        }

        if(vs.fix_outer_frame())
        {
            results_availability = vs.solve();
        }

        if(!state_holding)
        {
            vs.display(results_availability);
        }

        // 64bitのLinuxにおいて取得されるキーコードがおかしくなるため、256との剰余を取る。
        switch(waitKey(WAIT_TIME) % 256)
        {
            case CODE_ESCAPE:
                return EXIT_SUCCESS;

            case CODE_SPACE:
                state_holding = !state_holding;
                break;
        }
    }

    return EXIT_SUCCESS;
}
