cmake_minimum_required(VERSION 3.20)

project(BarCode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(barcode
    main.cpp
    BarCode/Core.h
    BarCode/CoreInternal.h
    BarCode/Core.cpp
    BarCode/Encoder.cpp
    BarCode/Gs1.cpp
    BarCode/Gtin.cpp
    BarCode/Image.h
    BarCode/Image.cpp
)

target_include_directories(barcode
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/BarCode
)

target_link_libraries(barcode
    PRIVATE
        Qt6::Core
        Qt6::Gui
)

if(MSVC)
    target_compile_options(barcode PRIVATE /W4 /permissive-)
else()
    target_compile_options(barcode PRIVATE -Wall -Wextra -Wpedantic)
endif()
