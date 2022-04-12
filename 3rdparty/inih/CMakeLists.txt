project(inih)

set(HEADER_FILES src/ini.h)
set(SOURCE_FILES src/ini.c)

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

