cmake_minimum_required(VERSION 3.8)

project(virtuawin-bar)

set(CMAKE_CXX_STANDARD 14)

set(HEADER_FILES
    src/VirtuaWin/defines.h
    src/VirtuaWin/messages.h
    src/config.h
)
set(SOURCE_FILES
    src/config.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${HEADER_FILES} ${SOURCE_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${HEADER_FILES} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)

find_package(Cairo QUIET)
if (Cairo_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC Cairo)
else()
    # Try to use the vcpkg version of Cairo.
    find_package(unofficial-cairo CONFIG QUIET)
    if (unofficial-cairo_FOUND)
        target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::cairo::cairo)
    else()
        # No Cairo, so trigger an error.
        find_package(Cairo REQUIRED)
    endif()
endif()
