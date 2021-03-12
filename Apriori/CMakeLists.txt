cmake_minimum_required(VERSION 3.17)
project(Apriori)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O3")

include_directories(.)

add_executable(Apriori
        Apriori.cpp
        Apriori.h
        HashTree.h)
