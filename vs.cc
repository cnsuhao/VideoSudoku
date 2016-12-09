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

int main(int argc, char **argv)
{
    VideoSudoku vs;

    bool results_availability = false;
    int code = vs.initialize(400, 0);

    switch(code)
    {
        case 1:
            ERROR("The camera device wasn't able to be opened.");

            return EXIT_FAILURE;

        case 2:
            ERROR("The OCR initialization was failed.");

            return EXIT_FAILURE;

        case 3:
            ERROR("The model file wasn't able to be opened.");

            return EXIT_FAILURE;

        default:
            break;
    }

    while(true)
    {
        if(!vs.capture_video())
        {
            ERROR("The camera device wasn't able to be read.");

            vs.finalize();

            return EXIT_FAILURE;
        }

        if(vs.fix_outer_frame())
        {
            results_availability = vs.solve();
        }

        vs.display(results_availability);

        // ESCキーの入力を取得する。
        // 64bitのLinuxにおいて取得されるキーコードがおかしくなるため、256との剰余を取る。
        if((waitKey(60) % 256) == 27)
        {
            break;
        }
    }

    vs.finalize();

    return EXIT_SUCCESS;
}
