cmake_minimum_required(VERSION 3.10)
project(tda)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(AzgraCppLibrary)
add_executable(tda main.cpp Stopwatch.cpp automata.h regex.cpp
        dis/SgmlFile.cpp dis/text_search.cpp dis/ReutersArticle.cpp
        dis/ReutersArticle.h dis/porter_stemmer.cpp dis/sgml_collection.cpp dis/sgml_collection.h)
target_link_libraries(tda sub::azgra)