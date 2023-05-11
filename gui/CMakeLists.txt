cmake_minimum_required(VERSION 3.5)

project(gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(../common ads/src)

add_subdirectory(ads)

add_library(gui OBJECT
  tmainwindow.cpp
  tmainwindow.h
  toscilloscopewidget.cpp
  toscilloscopewidget.h
  tconfigparamwidget.cpp
  tconfigparamwidget.h
  tfilenameedit.cpp
  tfilenameedit.h
  ttimeedit.cpp
  ttimeedit.h
)

target_link_libraries(gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(gui PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)
