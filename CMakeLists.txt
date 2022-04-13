cmake_minimum_required(VERSION 3.15)

project(virtuawin-bar)

option(HOT_RELOAD "Experimental hot reload support." OFF)

set(CMAKE_CXX_STANDARD 14)

set(DLL_HEADER_FILES
    src/VirtuaWin/defines.h
    src/VirtuaWin/messages.h
    src/bar.h
    src/config.h
    src/handler.h
    src/movement.h
)
set(DLL_SOURCE_FILES
    src/bar.cpp
    src/config.cpp
    src/handler.cpp
    src/movement.cpp
)
set(EXE_SOURCE_FILES
    src/main.cpp
)

# Disable exceptions and RTTI.
string(REGEX REPLACE "/EH[a-z]+" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (HOT_RELOAD)
    set(CORE ${PROJECT_NAME}-dll)
    add_library(${PROJECT_NAME}-dll SHARED
                ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES})
    target_compile_definitions(${PROJECT_NAME}-dll PRIVATE VWBAR_EXPORTS)
    target_compile_definitions(${PROJECT_NAME}-dll PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME}-dll PRIVATE /W4)
    set_target_properties(${PROJECT_NAME}-dll PROPERTIES OUTPUT_NAME vwbar)

    add_executable(${PROJECT_NAME} WIN32 ${EXE_SOURCE_FILES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE VWBAR_HOT_RELOAD)
else()
    set(CORE ${PROJECT_NAME})
    add_executable(${PROJECT_NAME} WIN32
                   ${EXE_SOURCE_FILES} ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES})
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)

source_group(TREE ${PROJECT_SOURCE_DIR}
             FILES ${DLL_HEADER_FILES} ${DLL_SOURCE_FILES} ${EXE_SOURCE_FILES})


###############
# Third-party #
###############

target_link_libraries(${CORE} PRIVATE dwmapi shlwapi)

# Use our bundled Cairo.
add_subdirectory(3rdparty/Cairo)
target_link_libraries(${CORE} PUBLIC Cairo)

# Use our bundled third-party INI reader.
add_subdirectory(3rdparty/inih)
target_link_libraries(${CORE} PUBLIC inih)
