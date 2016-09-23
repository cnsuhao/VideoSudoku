//!
//! @file      vs.cc
//! @brief     メインエントリ 実装
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#include "VideoSudoku.h"

int main(int argc, char **argv)
{
    bool results_availability = false;
    VideoSudoku vs;

    if(vs.initialize(400, 0) != 0)
    {
        return 1;
    }

    while(true)
    {
        if(!vs.capture_video())
        {
            return 1;
        }

        if(vs.fix_outer_frame())
        {
            results_availability = vs.solve();
        }

        vs.display(results_availability);

        // ESCキーの入力を取得する
        // 64bitのLinuxにおいて、取得されるキーコードがおかしくなるため256との剰余を取る
        if((waitKey(60) % 256) == 27)
        {
            break;
        }
    }

    vs.finalize();

    return 0;
}
