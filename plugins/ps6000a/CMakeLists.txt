cmake_minimum_required(VERSION 3.14)

project(TPS6000a LANGUAGES CXX)

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

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_NAME}_global.h
    tps6000a.cpp
    tps6000a.h
    tps6000ascope.h
    tps6000ascope.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ../../common)
target_include_directories(${PROJECT_NAME} PRIVATE "${PICOSCOPESDK_INCLUDE_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE "${PICOSCOPE6000A_LIBRARY}")

string(TOUPPER ${PROJECT_NAME}_LIBRARY project_library)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${project_library})
