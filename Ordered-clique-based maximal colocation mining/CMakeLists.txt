cmake_minimum_required(VERSION 3.17)
project("Ordered-clique-based maximal colocation mining")

set(CMAKE_CXX_STANDARD 14)

add_executable(Ordered-clique-based main.cpp TreeNode.cpp TreeNode.h PmTree.cpp PmTree.h InsTree.cpp InsTree.h algorithm.cpp)