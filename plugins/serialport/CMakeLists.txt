cmake_minimum_required(VERSION 3.14)

project(SerialPort LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

find_package(Qt6 REQUIRED COMPONENTS SerialPort)


include_directories(../../common)

add_library(TSerialPort SHARED
  TSerialPort_global.h
  tserialport.cpp
  tserialport.h
  tserialportdevice.h
  tserialportdevice.cpp
)

target_link_libraries(TSerialPort PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_link_libraries(TSerialPort PRIVATE Qt6::SerialPort)

target_compile_definitions(TSerialPort PRIVATE TSERIALPORT_LIBRARY)
