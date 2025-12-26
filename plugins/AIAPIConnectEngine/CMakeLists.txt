cmake_minimum_required(VERSION 3.14)

project(TAIAPIConnectEngine LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
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

add_library(TAIAPIConnectEngine SHARED
  AIAPIConnectEngine_global.h
  taiapiconnectengine.cpp
  taiapiconnectengine.h
  taiapiconnectenginedevice.h taiapiconnectenginedevice.cpp
  taiapiconnectenginedeviceinputstream.h taiapiconnectenginedeviceinputstream.cpp
  taiapiconnectenginedeviceoutputstream.h taiapiconnectenginedeviceoutputstream.cpp
  taiapiconnectenginedeviceaction.h taiapiconnectenginedeviceaction.cpp
)

target_link_libraries(TAIAPIConnectEngine PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(TAIAPIConnectEngine PRIVATE Qt${QT_VERSION_MAJOR}::Network)

if (TARGET hdf5_cpp-shared)
    target_link_libraries(TAIAPIConnectEngine PRIVATE hdf5_cpp-shared)
elseif (TARGET hdf5_cpp-static)
    target_link_libraries(TAIAPIConnectEngine PRIVATE hdf5_cpp-static)
else()
    message(FATAL_ERROR "No in-tree HDF5 C++ target (hdf5_cpp-*) found. Did top-level add_subdirectory(hdf5)?")
endif()

target_compile_definitions(TAIAPIConnectEngine PRIVATE AIAPICONNECTENGINE_LIBRARY)
