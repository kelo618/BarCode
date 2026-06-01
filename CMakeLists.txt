cmake_minimum_required(VERSION 3.20)
project(BarCode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BARCODE_BUILD_DEMO "Build demo executable" ON)
option(BARCODE_BUILD_TESTS "Build regression tests" ON)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)

set(BARCODE_CORE_SOURCES
    BarCode/BarCode.cpp
    BarCode/code128.cpp
    BarCode/code39.cpp
    BarCode/ean13.cpp
    BarCode/ean8.cpp
)

set(BARCODE_CORE_HEADERS
    BarCode/BarCode.h
    BarCode/BarcodeFactory.h
    BarCode/code128.h
    BarCode/code39.h
    BarCode/ean13.h
    BarCode/ean8.h
)

add_library(barcode_core STATIC ${BARCODE_CORE_SOURCES} ${BARCODE_CORE_HEADERS})
target_include_directories(barcode_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/BarCode
        ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(barcode_core PUBLIC ${OpenCV_LIBS})

if(BARCODE_BUILD_DEMO)
    add_executable(barcode_demo BarCodeDemo/main.cpp)
    target_link_libraries(barcode_demo PRIVATE barcode_core)
endif()

if(BARCODE_BUILD_TESTS)
    add_executable(barcode_tests BarCodeTests/BarCodeTests.cpp)
    target_link_libraries(barcode_tests PRIVATE barcode_core)
endif()
