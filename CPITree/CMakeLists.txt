cmake_minimum_required(VERSION 3.17)
project(CPITree)

set(CMAKE_CXX_STANDARD 14)

add_executable(CPITree main.cpp CPITree.cpp CPITree.h Types.h CPITreeNode.cpp CPITreeNode.h)