cmake_minimum_required(VERSION 3.14)
project(AccountsUiPlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME accounts_ui
    REP_FILE src/AccountsBackend.rep
    SOURCES
        src/AccountsInterface.h
        src/AccountsPlugin.h
        src/AccountsPlugin.cpp
        src/AccountsBackend.h
        src/AccountsBackend.cpp
)
