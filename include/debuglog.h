//!
//! @file  debuglog.h
//! @brief debuglog モジュール定義
//!

#pragma once

#include <stdio.h>

//! @brief エラーメッセージを表示する
#define ERROR(fmt, ...) do { printf("[ERROR] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)

//! @brief デバッグメッセージを表示する
#define DEBUG(fmt, ...) do { printf("[DEBUG] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)

//! @brief ログを表示する
#define LOG(fmt, ...) do { printf("[LOG] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)
