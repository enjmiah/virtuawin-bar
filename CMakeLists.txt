cmake_minimum_required(VERSION 3.8)

project(virtuawin-tiling)

option(HOT_RELOAD "Experimental hot reload support." OFF)

set(CMAKE_CXX_STANDARD 14)

set(DLL_HEADER_FILES
    src/VirtuaWin/defines.h
    src/VirtuaWin/messages.h
    src/bar.h
    src/config.h
    src/handler.h
    src/logging.h
    src/tiling.h
)
set(DLL_SOURCE_FILES
    src/bar.cpp
    src/config.cpp
    src/handler.cpp
    src/tiling.cpp
)
set(EXE_SOURCE_FILES
    src/main.cpp
)

if (HOT_RELOAD)
    set(CORE ${PROJECT_NAME}-dll)
    add_library(${PROJECT_NAME}-dll SHARED
                ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES})
    target_compile_definitions(${PROJECT_NAME}-dll PRIVATE VWTILING_EXPORTS)
    target_compile_definitions(${PROJECT_NAME}-dll PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    set_target_properties(${PROJECT_NAME}-dll PROPERTIES OUTPUT_NAME vwtiling)

    add_executable(${PROJECT_NAME} WIN32 ${EXE_SOURCE_FILES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE VWTILING_HOT_RELOAD)
else()
    set(CORE ${PROJECT_NAME})
    add_executable(${PROJECT_NAME} WIN32
                   ${EXE_SOURCE_FILES} ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)

source_group(TREE ${PROJECT_SOURCE_DIR}
             FILES ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES} ${EXE_SOURCE_FILES})

target_link_libraries(${CORE} PRIVATE dwmapi)
find_package(Cairo QUIET)
if (Cairo_FOUND)
    target_link_libraries(${CORE} PUBLIC Cairo)
else()
    # Try to use the vcpkg version of Cairo.
    find_package(unofficial-cairo CONFIG QUIET)
    if (unofficial-cairo_FOUND)
        target_link_libraries(${CORE} PUBLIC unofficial::cairo::cairo)
    else()
        # No Cairo, so trigger an error.
        find_package(Cairo REQUIRED)
    endif()
endif()
