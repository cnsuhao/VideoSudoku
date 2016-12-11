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

int main(int argc, char **argv)
{
    VideoSudoku vs;

    bool results_availability = false;

    switch(vs.initialize(WINDOW_SIZE, CAMERA_ID))
    {
        case 1:
            ERROR("The camera device wasn't able to be opened.\n");

            return EXIT_FAILURE;

        case 2:
            ERROR("The OCR initialization was failed.\n");

            return EXIT_FAILURE;

        case 3:
            ERROR("The model file wasn't able to be opened.\n");

            return EXIT_FAILURE;
    }

    while(true)
    {
        if(!vs.capture_video())
        {
            ERROR("The camera device wasn't able to be read.\n");

            return EXIT_FAILURE;
        }

        if(vs.fix_outer_frame())
        {
            results_availability = vs.solve();
        }

        vs.display(results_availability);

        // ESCキーの入力を取得する。
        // 64bitのLinuxにおいて取得されるキーコードがおかしくなるため、256との剰余を取る。
        if((waitKey(WAIT_TIME) % 256) == CODE_ESCAPE)
        {
            break;
        }
    }

    return EXIT_SUCCESS;
}
