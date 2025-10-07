cmake_minimum_required(VERSION 3.25)

project(caster_qt LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui 3DExtras Bluetooth Widgets)
message("Found Qt? ${Qt6_FOUND}")

find_library(CAST_SDK_BINARY cast PATHS ${CMAKE_SOURCE_DIR}/../../lib)
message("Clarius Cast SDK binary location: ${CAST_SDK_BINARY}")
add_library(CAST_SDK UNKNOWN IMPORTED)
set_target_properties(CAST_SDK PROPERTIES
    IMPORTED_LOCATION ${CAST_SDK_BINARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../../include
)

qt_add_executable(caster_qt
    3d.cpp
    3d.h
    caster.cpp
    caster.h
    caster.qrc
    caster.ui
    display.cpp
    display.h
    main.cpp
)

set_target_properties(caster_qt PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/Info.plist
    MACOSX_BUNDLE_GUI_IDENTIFIER "me.clarius.sdk.caster_qt"
)

target_precompile_headers(caster_qt PRIVATE pch.h)

target_link_libraries(caster_qt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::3DExtras
    Qt::Widgets
    CAST_SDK
)
