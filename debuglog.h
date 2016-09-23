#pragma once

#include <stdio.h>

#define ERROR(fmt, ...) do { printf("[ERROR] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)
#define DEBUG(fmt, ...) do {printf("[DEBUG] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)
#define LOG(fmt, ...) do { printf("[LOG] %s(%d) : " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__); } while(0)
