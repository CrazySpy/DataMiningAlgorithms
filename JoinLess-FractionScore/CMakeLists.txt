cmake_minimum_required(VERSION 3.17)
project(JoinLess)

set(CMAKE_CXX_STANDARD 14)

add_executable(JoinLess_FractionScore main.cpp JoinLess.cpp JoinLess.h FractionScore.cpp FractionScore.h CSVReader/CSVReader.h CSVReader/CSVReader.cpp)