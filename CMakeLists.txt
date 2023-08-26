# -------------------------------------------------
# Project created by QtCreator 2010-07-09T14:22:28
# -------------------------------------------------
# ported to CMake 2023-08-25

cmake_minimum_required(VERSION 3.1)
project(classphoto VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS
  Widgets
  Core
  Gui
  Xml
  Widgets
  PrintSupport
REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DISABLE_DEPRECATED_UP_TO=0x050F00")

set(SOURCES main.cpp
    mainwindow.cpp
    project.cpp
    studentbase.cpp
    studentlistitemdelegate.cpp
    availableitemsmodel.cpp
    picture.cpp
    editshowclassname.cpp
    classnameingraphicsview.cpp
    studentingraphicsview.cpp
    studentsview.cpp
    studenteditdialog.cpp
    studentnamedialog.cpp
    croppicture.cpp
    croprenderer.cpp
    createclassdlg.cpp
    settings.cpp
)

qt6_add_resources(SOURCES icons.qrc)

add_executable(classphoto ${SOURCES})
target_link_libraries(classphoto
  Qt6::Widgets
  Qt6::Gui
  Qt6::Xml
  Qt6::PrintSupport
)
