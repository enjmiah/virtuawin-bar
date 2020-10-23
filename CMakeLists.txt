cmake_minimum_required(VERSION 3.8)

project(virtuawin-bar)

option(USE_VCPKG_CAIRO "Use the version of Cairo packaged by the vcpkg package manager" OFF)

set(CMAKE_CXX_STANDARD 14)

set(HEADER_FILES
    src/VirtuaWin/defines.h
    src/VirtuaWin/messages.h
    src/config.h)
set(SOURCE_FILES
    src/config.cpp
    src/main.cpp)

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${HEADER_FILES} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)

if (USE_VCPKG_CAIRO)
    find_package(unofficial-cairo CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::cairo::cairo)
else()
    find_package(Cairo REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Cairo)
endif()

find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
