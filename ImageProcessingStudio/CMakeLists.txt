cmake_minimum_required(VERSION 3.16)
project(ImageProcessingStudio VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(ImageProcessingStudio
    main.cpp
    mainwindow.cpp
    mainwindow.h
    filters.cpp
    Image_Class.h
    gif.h
    CanvasWidget.h
    stb_image_impl.cpp
    CanvasWidget.cpp
)

target_link_libraries(ImageProcessingStudio
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
