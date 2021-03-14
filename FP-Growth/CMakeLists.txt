cmake_minimum_required(VERSION 3.17)
project(FP_Growth)

set(CMAKE_CXX_STANDARD 14)

include_directories(FP-Growth)

add_executable(FP_Growth
        main.cpp
        FPGrowth.h
        FPTree.h)
