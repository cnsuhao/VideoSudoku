cmake_minimum_required(VERSION 3.7)

project("videosudoku" C CXX)

set(CMAKE_C_FLAGS "-Wall -Wextra -std=gnu99 -O3")
set(CMAKE_C_FLAGS_DEBUG "-g")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14 -DAPP_MAIN")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DVIDEOSUDOKU_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(INSTALL "${CMAKE_CURRENT_SOURCE_DIR}/resource" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB_RECURSE c_sourses RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c")
file(GLOB_RECURSE cxx_sourses RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cc")

set(sources ${c_sourses} ${cxx_sourses})

find_package(OpenCV REQUIRED)

include_directories("include" ${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} "svm")
