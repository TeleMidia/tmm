cmake_minimum_required (VERSION 2.6)

project(tmm-gui)

# CMake/Qt Integration
set(CMAKE_INCLUDE_CURRENTDIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Xml Widgets)

add_executable(tmm-gui
  TMMWindow.h
  TreeModel.h
  main.cpp
  TMMWindow.cpp
  TreeModel.cpp

  TMMWindow.ui
  resources.qrc  
)

target_include_directories(tmm-gui PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  .
)

target_link_libraries(tmm-gui
  Qt5::Core
  Qt5::Xml
  Qt5::Widgets
)

