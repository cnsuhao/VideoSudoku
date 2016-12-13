//!
//! @file      SudokuOCR.cc
//! @brief     SudokuOCR ファクトリ実装
//!

#include "SudokuOCR.h"
#include "SVMOCR.h"

SudokuOCR *sudokuOCRFactory(const char *class_name)
{
    return new SVMOCR();
}
