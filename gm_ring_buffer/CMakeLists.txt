cmake_minimum_required(VERSION 3.10)

# 定义静态库
add_library(gm_ring_buffer STATIC gm_ring_buffer.c)

# 添加头文件搜索路径
target_include_directories(gm_ring_buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
