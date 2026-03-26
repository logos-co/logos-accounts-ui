cmake_minimum_required(VERSION 3.14)
project(AccountsUiPlugin LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME accounts_ui
    SOURCES
        src/AccountsPlugin.h
        src/AccountsPlugin.cpp
        src/AccountsBackend.h
        src/AccountsBackend.cpp
        src/accounts_resources.qrc
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

# Qt Widgets, Quick, and QuickWidgets are required for QML-based UI but not
# included by logos_module() — find and link them explicitly.
find_package(Qt6 REQUIRED COMPONENTS Widgets RemoteObjects Quick QuickWidgets)
target_link_libraries(accounts_ui_module_plugin PRIVATE
    Qt6::Widgets
    Qt6::RemoteObjects
    Qt6::Quick
    Qt6::QuickWidgets
)
