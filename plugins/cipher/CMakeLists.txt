cmake_minimum_required(VERSION 3.14)

project(TCipherPlugin LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_NAME}_global.h
    tcipherplugin.cpp
    tcipherplugin.h
    taesengine.cpp
    taesengine.h
    tcipheraction.cpp
    tcipheraction.h
    tcipherinputstream.cpp
    tcipherinputstream.h
    tcipheroutputstream.cpp
    tcipheroutputstream.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ../../common)
target_include_directories(${PROJECT_NAME} PRIVATE ../common)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)

string(TOUPPER ${PROJECT_NAME}_LIBRARY project_library)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${project_library})
