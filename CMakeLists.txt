cmake_minimum_required(VERSION 3.8)

project(virtuawin-bar)

set(CMAKE_CXX_STANDARD 14)

set(DLL_HEADER_FILES
    src/VirtuaWin/defines.h
    src/VirtuaWin/messages.h
    src/bar.h
    src/config.h
    src/handler.h
)
set(DLL_SOURCE_FILES
    src/bar.cpp
    src/config.cpp
    src/handler.cpp
)
set(EXE_SOURCE_FILES
    src/main.cpp
)

add_library(${PROJECT_NAME}-dll SHARED ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME}-dll PRIVATE VWBAR_EXPORTS)
target_compile_definitions(${PROJECT_NAME}-dll PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
set_target_properties(${PROJECT_NAME}-dll PROPERTIES OUTPUT_NAME vwbar)

add_executable(${PROJECT_NAME} WIN32 ${EXE_SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-dll)

source_group(TREE ${PROJECT_SOURCE_DIR}
             FILES ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES} ${EXE_SOURCE_FILES})

find_package(Cairo QUIET)
if (Cairo_FOUND)
    target_link_libraries(${PROJECT_NAME}-dll PUBLIC Cairo)
else()
    # Try to use the vcpkg version of Cairo.
    find_package(unofficial-cairo CONFIG QUIET)
    if (unofficial-cairo_FOUND)
        target_link_libraries(${PROJECT_NAME}-dll PUBLIC unofficial::cairo::cairo)
    else()
        # No Cairo, so trigger an error.
        find_package(Cairo REQUIRED)
    endif()
endif()
