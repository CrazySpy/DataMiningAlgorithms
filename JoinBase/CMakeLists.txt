cmake_minimum_required(VERSION 3.17)
project(JoinBase)

set(CMAKE_CXX_STANDARD 14)

add_executable(JoinBase main.cpp Types.h Types.cpp JoinBase.cpp JoinBase.h MultiResolution.cpp MultiResolution.h CSVReader/CSVReader.h CSVReader/CSVReader.cpp)