cmake_minimum_required(VERSION 3.10)
project(tda)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(AzgraCppLibrary)
add_executable(tda main.cpp Stopwatch.cpp automata.h regex.cpp dis/SgmlFile.cpp text_search.h dis/ReutersArticle.cpp dis/ReutersArticle.h)
target_link_libraries(tda sub::azgra)