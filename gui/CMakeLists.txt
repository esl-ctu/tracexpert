cmake_minimum_required(VERSION 3.5)

project(gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Xml)

add_subdirectory(ads)

add_library(${PROJECT_NAME} OBJECT
  tdockmanager.cpp
  tdockmanager.h
  tmainwindow.cpp
  tmainwindow.h
  tscopewidget.cpp
  tscopewidget.h
  tconfigparamwidget.cpp
  tconfigparamwidget.h
  tfilenameedit.cpp
  tfilenameedit.h
  ttimeedit.cpp
  ttimeedit.h
  tiodevicewidget.cpp
  tiodevicewidget.h
  tdevicewizard.cpp
  tdevicewizard.h
  tdialog.cpp
  tdialog.h
  tiodevicemodel.cpp
  tiodevicemodel.h
  tcomponentmodel.cpp
  tcomponentmodel.h
  tscopemodel.cpp
  tscopemodel.h
  tpluginunitmodel.cpp
  tpluginunitmodel.h
  tpluginunitcontainer.cpp
  tpluginunitcontainer.h
  tpluginunitcontainerview.cpp
  tpluginunitcontainerview.h
  tcomponentcontainer.cpp
  tcomponentcontainer.h
  tiodevicecontainer.cpp
  tiodevicecontainer.h
  tscopecontainer.cpp
  tscopecontainer.h
  tprojectmodel.cpp
  tprojectmodel.h
  tprojectview.cpp
  tprojectview.h
  tprojectitem.cpp
  tprojectitem.h
  protocol/tabstracttablemodel.h
  protocol/tprotocolwidget.cpp
  protocol/tprotocolwidget.h
  protocol/tprotocoleditor.cpp
  protocol/tprotocoleditor.h
  protocol/tmessagesimplecontainer.h
  protocol/tmessageeditor.h
  protocol/tmessageeditor.cpp
  protocol/tmessagepartsimplecontainer.h
  protocol/tmessageparteditor.h
  protocol/tprotocolcontainer.h
  protocol/tprotocolcontainer.cpp
  protocol/tprotocolmodel.h
  protocol/tprotocolmodel.cpp
  protocol/tmessageparteditor.cpp
  protocol/tprotocoltableview.h
  tmessageformmanager.h
  tmessageformmanager.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ads/src)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Charts)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Xml)

target_link_libraries(${PROJECT_NAME} PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)
