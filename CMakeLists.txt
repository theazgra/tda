cmake_minimum_required(VERSION 3.10)
project(tda CXX)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
# set(CMAKE_CXX_STANDARD 17)


add_subdirectory(AzgraCppLibrary)


add_executable(tda main.cpp Stopwatch.cpp automata.h regex.cpp
        dis/SgmlFile.cpp dis/text_search.cpp dis/ReutersArticle.cpp
        dis/ReutersArticle.h dis/porter_stemmer.cpp dis/sgml_collection.cpp dis/sgml_collection.h
        dis/vector_model.cpp
        dis/document_clusterer.cpp)

target_link_libraries(tda PRIVATE azgra)
set_property(TARGET tda PROPERTY CXX_STANDARD 17)

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message("------- OpenMP ENABLED -------")
    target_link_libraries(tda PRIVATE OpenMP::OpenMP_CXX)
endif()
