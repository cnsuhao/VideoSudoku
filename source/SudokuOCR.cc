//!
//! @file  SudokuOCR.cc
//! @brief SudokuOCR ファクトリ実装
//!

#include "SudokuOCR.h"

#include "SVMOCR.h"

namespace videosudoku
{
SudokuOCR *sudokuOCRFactory(const char *)
{
    return new SVMOCR();
}
}
