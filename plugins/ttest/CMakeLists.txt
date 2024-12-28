cmake_minimum_required(VERSION 3.14)

project(TTestPlugin LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

include_directories(../../common)
include_directories(../common)

add_library(TTTestPlugin SHARED
  TTTestPlugin_global.h
  tttestplugin.cpp
  tttestplugin.h
  tttestdevice.cpp
  tttestdevice.h
  tttestaction.cpp
  tttestaction.h
  tttestinputstream.cpp
  tttestinputstream.h
  tttestoutputstream.cpp
  tttestoutputstream.h
)

target_link_libraries(TTTestPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(TTTestPlugin PRIVATE TTTESTPLUGIN_LIBRARY)
