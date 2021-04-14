cmake_minimum_required(VERSION 3.19)
project(Differ)

set(CMAKE_CXX_STANDARD 17)

add_executable(Differ main.cpp DiffMaker.h DiffMaker.cpp)