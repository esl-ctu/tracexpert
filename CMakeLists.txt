cmake_minimum_required(VERSION 3.5)

project(TraceXpert VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Xml Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Xml Concurrent)

add_subdirectory(ads)

add_executable(${PROJECT_NAME}
    application.qrc
    main.cpp
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
    tcombobox.h tcombobox.cpp
    tdevicemodel.h
    tdevicemodel.cpp
    treceiver.h treceiver.cpp
    tsender.h tsender.cpp
    tanaldevicemodel.h tanaldevicemodel.cpp
    tanaldevicecontainer.h tanaldevicecontainer.cpp
    tanalstreamreceiver.h tanalstreamreceiver.cpp
    tiodevicereceiver.h tiodevicereceiver.cpp
    tiodevicesender.h tiodevicesender.cpp
    tanalstreamsender.h tanalstreamsender.cpp
    tsendermodel.h tsendermodel.cpp
    treceivermodel.h treceivermodel.cpp
    tanalactionrunner.h tanalactionrunner.cpp
    tanalactionaborter.h tanalactionaborter.cpp
    tanalactionmodel.h tanalactionmodel.cpp
    tanaldevicewidget.h tanaldevicewidget.cpp
    tanalstreamreceivermodel.h tanalstreamreceivermodel.cpp
    tanalstreamsendermodel.h tanalstreamsendermodel.cpp
    scenario/tscenariographicalconnection.h
    scenario/tscenariographicalconnection.cpp
    scenario/tscenariographicalitem.h
    scenario/tscenariographicalitem.cpp
    scenario/tscenariographicalitemport.h
    scenario/tscenariographicalitemport.cpp
    scenario/tscenariocontainer.h
    scenario/tscenariocontainer.cpp
    scenario/tscenarioeditorwidget.h
    scenario/tscenarioeditorwidget.cpp
    scenario/tscenariomodel.h
    scenario/tscenariomodel.cpp
    scenario/tscenarioscene.h
    scenario/tscenarioscene.cpp
    scenario/tscenariowidget.h
    scenario/tscenariowidget.cpp
    scenario/tscenarioexecutor.h
    scenario/tscenarioexecutor.cpp
    scenario/tscenarioitem.h
    scenario/tscenarioitem.cpp
    scenario/tscenarioitemport.h
    scenario/tscenarioconnection.h
    scenario/tscenariographicsview.h
    scenario/tscenariographicsview.cpp
    scenario/graphical_items/tscenariographicalembeddedsubtitleitem.h
    scenario/scenario_items/tscenariodelayitem.h
    scenario/scenario_items/tscenarioprotocolencodeitem.h
)

target_include_directories(${PROJECT_NAME} PRIVATE common)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Charts)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Xml)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)

target_link_libraries(${PROJECT_NAME} PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)

function (add_plugin dir target)
  add_subdirectory(plugins/${dir})
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:${target}>"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins")
endfunction()

add_plugin(analtestingplugin TAnalTestingPlugin)
add_plugin(cpa TCPAPlugin)
add_plugin(dummyscope TDummyScopePlugin)
add_plugin(emptyplugin TEmptyPlugin)
add_plugin(file TFile)
add_plugin(newplugin TNewPlugin)
add_plugin(ps6000 TPS6000)
add_plugin(ps6000a TPS6000a)
add_plugin(random TRandom)
add_plugin(serialport TSerialPort)
add_plugin(smartcard TSmartCard)
add_plugin(ttest TTTestPlugin)
