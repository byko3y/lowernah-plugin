cmake_minimum_required(VERSION 3.10)
project(lowernah LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml)

# Path to <psi source>/plugins/include
set(PSI_PLUGIN_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/psi/src/plugins/include" CACHE PATH "Directory with psiplugin.h")

add_library(lowernah MODULE lowernah.cpp)
target_include_directories(lowernah PRIVATE ${PSI_PLUGIN_INCLUDE})
target_link_libraries(lowernah Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml)

