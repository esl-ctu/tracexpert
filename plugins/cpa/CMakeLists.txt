cmake_minimum_required(VERSION 3.14)

project(CPAPlugin LANGUAGES CXX)

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

add_library(TCPAPlugin SHARED
  TCPAPlugin_global.h
  tcpaplugin.cpp
  tcpaplugin.h
  tcpadevice.cpp
  tcpadevice.h
  tcpaaction.cpp
  tcpaaction.h
  tcpainputstream.cpp
  tcpainputstream.h
  tcpaoutputstream.cpp
  tcpaoutputstream.h
)

target_link_libraries(TCPAPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(TCPAPlugin PRIVATE TCPAPLUGIN_LIBRARY)
