cmake_minimum_required(VERSION 3.14)

project(TPS6000A LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

find_package(PicoScopeSDK REQUIRED)

include_directories(../../common)
include_directories("${PICOSCOPESDK_INCLUDE_DIR}")

add_library(TPS6000a SHARED
  TPS6000a_global.h
  tps6000a.cpp
  tps6000a.h
  tps6000ascope.h
  tps6000ascope.cpp
)

target_link_libraries(TPS6000a PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(TPS6000a PRIVATE "${PICOSCOPE6000A_LIBRARY}")

target_compile_definitions(TPS6000a PRIVATE TPS6000a_LIBRARY)
