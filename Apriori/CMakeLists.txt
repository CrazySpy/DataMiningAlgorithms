cmake_minimum_required(VERSION 3.17)
project(Apriori)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Apriori
        main.cpp
        Apriori.h
        HashTree.h)
