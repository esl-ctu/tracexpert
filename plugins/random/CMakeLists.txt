cmake_minimum_required(VERSION 3.14)

project(Random LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

include_directories(../../common)

add_library(TRandom SHARED
  TRandom_global.h
  trandom.cpp
  trandom.h
  trandomdevice.h
  trandomdevice.cpp
)

target_link_libraries(TRandom PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(TRandom PRIVATE TRANDOM_LIBRARY)
