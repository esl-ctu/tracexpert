cmake_minimum_required(VERSION 3.5)

project(TraceXpert VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(gui)

include_directories(common gui gui/ads/src)

add_executable(TraceXpert
        main.cpp
)

target_link_libraries(TraceXpert PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(TraceXpert PRIVATE gui)
