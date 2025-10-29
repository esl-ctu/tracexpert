cmake_minimum_required(VERSION 3.14)

project(AIAPIConnectEngine LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
#target_link_libraries(mytarget PRIVATE Qt6::Network)

include_directories(../../common)

add_library(AIAPIConnectEngine SHARED
  AIAPIConnectEngine_global.h
  taiapiconnectengine.cpp
  taiapiconnectengine.h
  taiapiconnectenginedevice.h taiapiconnectenginedevice.cpp
  taiapiconnectenginedeviceinputstream.h taiapiconnectenginedeviceinputstream.cpp
  taiapiconnectenginedeviceoutputstream.h taiapiconnectenginedeviceoutputstream.cpp
  taiapiconnectenginedeviceaction.h taiapiconnectenginedeviceaction.cpp
)

target_link_libraries(AIAPIConnectEngine PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(AIAPIConnectEngine PRIVATE Qt${QT_VERSION_MAJOR}::Network)

find_package(HDF5 REQUIRED COMPONENTS C CXX)
target_include_directories(AIAPIConnectEngine PRIVATE ${HDF5_INCLUDE_DIRS})
target_link_libraries(AIAPIConnectEngine PUBLIC HDF5::HDF5)

target_compile_definitions(AIAPIConnectEngine PRIVATE AIAPICONNECTENGINE_LIBRARY)
