cmake_minimum_required(VERSION 3.10)
project(tda)

set(CMAKE_CXX_STANDARD 17)

add_executable(tda main.cpp Stopwatch.cpp automata.h)