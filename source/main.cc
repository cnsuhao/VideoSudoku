//!
//! @file      main.cc
//! @brief     メインエントリ 実装
//! @author    Sakamoto Kanta
//! @author    Suzuki Shota
//! @date      2016/8/5
//! @copyright (c) 2016 Sakamoto Kanta, Suzuki Shota
//!

#include <opencv2/highgui.hpp>

#include "debuglog.h"
#include "VideoSudoku.h"

namespace
{
using namespace cv;
using namespace videosudoku;

constexpr auto result_size = 400; //!< 結果画像のサイズ
constexpr auto camera_id = 0;     //!< カメラデバイスのID
constexpr auto wait_time = 16;    //!< キー入力の待ち時間 (ms)
constexpr auto code_escape = 27;  //!< Escapeキーのキーコード
constexpr auto code_space = 32;   //!< Spaceキーのキーコード

//! @brief  VideoSudokuのインスタンスを初期化する
//! @param  videosudoku 初期化するインスタンス
//! @retval true  成功した場合
//! @retval false 失敗した場合
bool initialize(VideoSudoku &videoSudoku)
{
    const auto code = videoSudoku.initialize(result_size, camera_id);

    if(code != 0)
    {
        if(code == 1)
        {
            ERROR("The camera device wasn't able to be opened.");
        }
        else if(code == 2)
        {
            ERROR("The OCR initialization was failed.");
        }
        else if(code == 3)
        {
            ERROR("The model file wasn't able to be opened.");
        }

        return false;
    }

    return true;
}
}

#ifdef APP_MAIN
int main()
{
    VideoSudoku videoSudoku;

    if(!initialize(videoSudoku)) return 1;

    auto continuation = true;
    auto state_holding = false;

    while(continuation)
    {
        if(!state_holding)
        {
            if(!videoSudoku.capture_video())
            {
                continuation = false;
            }
        }

        videoSudoku.display(videoSudoku.solve());

        // 環境によって取得されるキーコードが変わるため、256との剰余を取る。
        const auto key = waitKey(wait_time) % 256;

        if(key == code_escape)
        {
            continuation = false;
        }
        else if(key == code_space)
        {
            state_holding = !state_holding;
        }
    }

    return 0;
}
#endif
