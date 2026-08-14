cmake_minimum_required(VERSION 3.20)

project(FactoryGraph)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FactoryGraph
    src/main.cpp
    src/Core/Resource.cpp 
    include/FactoryGraph/Core/Resource.h 
    src/Core/Buildings.cpp
    include/FactoryGraph/Core/Buildings.h 
    src/Core/Recipe.cpp 
    include/FactoryGraph/Core/Recipe.h
    src/Database/Database.cpp 
    include/FactoryGraph/Database/Database.h
    include/FactoryGraph/Utils/Utils.h
    src/Utils/Utils.cpp
    
    # SQLite3
    external/sqlite3/sqlite3.c
    external/sqlite3/sqlite3.h
    external/sqlite3/sqlite3ext.h
)

target_include_directories(
        FactoryGraph PRIVATE
        include
        external/sqlite3
)