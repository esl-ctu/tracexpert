cmake_minimum_required(VERSION 3.14)

project(TPS6000 LANGUAGES CXX)

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

message("${PICOSCOPESDK_INCLUDE_DIR}")

add_library(TPS6000 SHARED
  TPS6000_global.h
  tps6000.cpp
  tps6000.h
  tps6000scope.h
  tps6000scope.cpp
)

target_link_libraries(TPS6000 PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(TPS6000 PRIVATE "${PICOSCOPESDK_LIBRARY}")

message("${PICOSCOPESDK_LIBRARY}")

target_compile_definitions(TPS6000 PRIVATE TPS6000_LIBRARY)
