cmake_minimum_required(VERSION 3.14)

project(TSmartCard LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

include_directories(../../common)

add_library(TSmartCard SHARED
  TSmartCard_global.h
  tsmartcard.cpp
  tsmartcard.h
  tsmartcarddevice.h
  tsmartcarddevice.cpp
)

target_link_libraries(TSmartCard PRIVATE Qt${QT_VERSION_MAJOR}::Core)

find_library(WINSCARD_LIBRARY NAMES WinSCard)
target_link_libraries(TSmartCard PRIVATE "${WINSCARD_LIBRARY}")

target_compile_definitions(TSmartCard PRIVATE TSMARTCARD_LIBRARY)
