cmake_minimum_required(VERSION 3.20)
project(gc1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-fpermissive")

add_executable(gc1 main.cpp ObjectPrinter.h)
