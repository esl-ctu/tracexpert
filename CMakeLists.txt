cmake_minimum_required(VERSION 3.5)

project(TraceXpert VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Remove the 'lib' prefix for all libraries
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Ensure no 'lib' prefix is added to library names
set(CMAKE_SHARED_LIBRARY_PREFIX "")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Xml Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Xml Concurrent)

add_subdirectory(ads)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

add_executable(${PROJECT_NAME} WIN32
    ${APP_ICON_RESOURCE_WINDOWS}
    application.qrc
    main.cpp
    tdevicewizard.cpp
    tdevicewizard.h
    tdialog.cpp
    tdialog.h
    tdockmanager.cpp
    tdockmanager.h
    tmainwindow.cpp
    tmainwindow.h
    tmessageformmanager.cpp
    tmessageformmanager.h
    common/tanaldevice.h
    common/tcommon.h
    common/tconfigparam.h
    common/tiodevice.h
    common/tmessage.h
    common/tmessagepart.h
    common/tplugin.h
    common/tprotocol.h
    common/tscope.h
    common/widgets/tcodeedit.cpp
    common/widgets/tcodeedit.h
    common/widgets/tcombobox.cpp
    common/widgets/tcombobox.h
    common/widgets/tconfigparamwidget.cpp
    common/widgets/tconfigparamwidget.h
    common/widgets/tfilenameedit.cpp
    common/widgets/tfilenameedit.h
    common/widgets/ttimeedit.cpp
    common/widgets/ttimeedit.h
    logger/tloghandler.cpp
    logger/tloghandler.h
    logger/tloglinewidget.cpp
    logger/tloglinewidget.h
    pluginunit/tpluginunitcontainer.cpp
    pluginunit/tpluginunitcontainer.h
    pluginunit/tpluginunitcontainerview.cpp
    pluginunit/tpluginunitcontainerview.h
    pluginunit/tpluginunitmodel.cpp
    pluginunit/tpluginunitmodel.h
    pluginunit/anal/tanaldevicecontainer.cpp
    pluginunit/anal/tanaldevicecontainer.h
    pluginunit/anal/tanaldevicemodel.cpp
    pluginunit/anal/tanaldevicemodel.h
    pluginunit/anal/tanaldevicewidget.cpp
    pluginunit/anal/tanaldevicewidget.h
    pluginunit/anal/action/tanalactionaborter.cpp
    pluginunit/anal/action/tanalactionaborter.h
    pluginunit/anal/action/tanalactionmodel.cpp
    pluginunit/anal/action/tanalactionmodel.h
    pluginunit/anal/action/tanalactionrunner.cpp
    pluginunit/anal/action/tanalactionrunner.h
    pluginunit/anal/stream/tanalstreamreceiver.cpp
    pluginunit/anal/stream/tanalstreamreceiver.h
    pluginunit/anal/stream/tanalstreamreceivermodel.cpp
    pluginunit/anal/stream/tanalstreamreceivermodel.h
    pluginunit/anal/stream/tanalstreamsender.cpp
    pluginunit/anal/stream/tanalstreamsender.h
    pluginunit/anal/stream/tanalstreamsendermodel.cpp
    pluginunit/anal/stream/tanalstreamsendermodel.h
    pluginunit/common/tdevicemodel.cpp
    pluginunit/common/tdevicemodel.h
    pluginunit/common/receiver/treceiver.cpp
    pluginunit/common/receiver/treceiver.h
    pluginunit/common/receiver/treceivermodel.cpp
    pluginunit/common/receiver/treceivermodel.h
    pluginunit/common/sender/tsender.cpp
    pluginunit/common/sender/tsender.h
    pluginunit/common/sender/tsendermodel.cpp
    pluginunit/common/sender/tsendermodel.h
    pluginunit/component/tcomponentcontainer.cpp
    pluginunit/component/tcomponentcontainer.h
    pluginunit/component/tcomponentmodel.cpp
    pluginunit/component/tcomponentmodel.h
    pluginunit/io/tiodevicecontainer.cpp
    pluginunit/io/tiodevicecontainer.h
    pluginunit/io/tiodevicemodel.cpp
    pluginunit/io/tiodevicemodel.h
    pluginunit/io/tiodevicereceiver.cpp
    pluginunit/io/tiodevicereceiver.h
    pluginunit/io/tiodevicesender.cpp
    pluginunit/io/tiodevicesender.h
    pluginunit/io/tiodevicewidget.cpp
    pluginunit/io/tiodevicewidget.h
    pluginunit/scope/tscopecontainer.cpp
    pluginunit/scope/tscopecontainer.h
    pluginunit/scope/tscopemodel.cpp
    pluginunit/scope/tscopemodel.h
    pluginunit/scope/tscopewidget.cpp
    pluginunit/scope/tscopewidget.h
    project/tprojectitem.cpp
    project/tprojectitem.h
    project/tprojectmodel.cpp
    project/tprojectmodel.h
    project/tprojectview.cpp
    project/tprojectview.h
    protocol/tabstracttablemodel.h
    protocol/tmessageeditor.cpp
    protocol/tmessageeditor.h
    protocol/tmessageparteditor.cpp
    protocol/tmessageparteditor.h
    protocol/tmessagepartsimplecontainer.h
    protocol/tmessagesimplecontainer.h
    protocol/tprotocolcontainer.cpp
    protocol/tprotocolcontainer.h
    protocol/tprotocoleditor.cpp
    protocol/tprotocoleditor.h
    protocol/tprotocolmodel.cpp
    protocol/tprotocolmodel.h
    protocol/tprotocoltableview.h
    protocol/tprotocolwidget.cpp
    protocol/tprotocolwidget.h
    scenario/tscenarioconnection.h
    scenario/tscenariocontainer.cpp
    scenario/tscenariocontainer.h
    scenario/tscenarioeditorwidget.cpp
    scenario/tscenarioeditorwidget.h
    scenario/tscenarioexecutor.cpp
    scenario/tscenarioexecutor.h
    scenario/tscenariographicalconnection.cpp
    scenario/tscenariographicalconnection.h
    scenario/tscenariographicalitem.cpp
    scenario/tscenariographicalitem.h
    scenario/tscenariographicalitemport.cpp
    scenario/tscenariographicalitemport.h
    scenario/tscenariographicsview.cpp
    scenario/tscenariographicsview.h
    scenario/tscenarioitem.cpp
    scenario/tscenarioitem.h
    scenario/tscenarioitemport.h
    scenario/tscenariomodel.cpp
    scenario/tscenariomodel.h
    scenario/tscenarioscene.cpp
    scenario/tscenarioscene.h
    scenario/tscenariowidget.cpp
    scenario/tscenariowidget.h
    scenario/graphical_items/tscenariographicalembeddedsubtitleitem.h
    scenario/scenario_items/tscenariodelayitem.h
    scenario/scenario_items/tscenariooutputfileitem.h
    scenario/scenario_items/tscenarioprotocolencodeitem.h
    scenario/scenario_items/tscenarioscopesingleitem.h
    scenario/scenario_items/tscenarioscopestartitem.cpp
    scenario/scenario_items/tscenarioscopestartitem.h
    scenario/scenario_items/tscenarioscopestopitem.cpp
    scenario/scenario_items/tscenarioscopestopitem.h
    scenario/scenario_items/tscenarioscriptitem.h
    scenario/scenario_items/tscenariovariablereaditem.h
    scenario/scenario_items/tscenariovariablewriteitem.h
    scenario/scenario_items/tscenarioanaldeviceactionitem.h
    scenario/scenario_items/tscenarioanaldevicereaditem.h
    scenario/scenario_items/tscenarioanaldevicewriteitem.h
    scenario/tscenarioexecutionexceptions.h
)

target_include_directories(${PROJECT_NAME} PRIVATE common)

target_link_libraries(${PROJECT_NAME} PRIVATE qt${QT_VERSION_MAJOR}advanceddocking)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Charts)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Xml)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)

function(add_plugin dir target additional_files)
  # Add the subdirectory for the plugin
  add_subdirectory(plugins/${dir})

  # Copy the library (.dll) file to the plugin directory
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:${target}>"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins")

  # Copy additional files (if provided) to the plugin directory (separate by ";")
  if(additional_files)
    foreach(file IN LISTS additional_files)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/plugins/${dir}/${file}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/${file}")
      endforeach()
    endif()
endfunction()


add_plugin(cipher TCipherPlugin "")
add_plugin(predict TPredictPlugin "")
add_plugin(cpa TCPAPlugin "")
add_plugin(dummyscope TDummyScopePlugin "")
add_plugin(file TFile "")
add_plugin(ps6000 TPS6000 "")
add_plugin(ps6000a TPS6000a "")
add_plugin(random TRandom "")
add_plugin(serialport TSerialPort "")
add_plugin(smartcard TSmartCard "")
add_plugin(newae Tnewae executable.py)
add_plugin(ttest TTTestPlugin "")
