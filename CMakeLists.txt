cmake_minimum_required(VERSION 3.20)

project(FactoryGraph)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FactoryGraph
    src/main.cpp
    src/Core/Resource.cpp 
    include/FactoryGraph/Resource.h 
    src/Core/Buildings.cpp
    include/FactoryGraph/Buildings.h 
    src/Core/Recipe.cpp 
    include/FactoryGraph/Recipe.h
)

target_include_directories(
    FactoryGraph PRIVATE include
)