project(pixman)

set(HEADER_FILES
    src/dither/blue-noise-64x64.h
    src/loongson-mmintrin.h
    src/pixman.h
    src/pixman-accessor.h
    src/pixman-arm-asm.h
    src/pixman-arm-common.h
    src/pixman-arm-neon-asm.h
    src/pixman-arm-simd-asm.h
    src/pixman-combine32.h
    src/pixman-compiler.h
    src/pixman-edge-imp.h
    src/pixman-inlines.h
    src/pixman-mips-dspr2.h
    src/pixman-mips-dspr2-asm.h
    src/pixman-version.h)

set(SOURCE_FILES
    src/dither/make-blue-noise.c
    src/pixman.c
    src/pixman-access.c
    src/pixman-access-accessors.c
    src/pixman-arm.c
    src/pixman-arm-neon.c
    src/pixman-arm-simd.c
    src/pixman-bits-image.c
    src/pixman-combine32.c
    src/pixman-combine-float.c
    src/pixman-conical-gradient.c
    src/pixman-edge.c
    src/pixman-edge-accessors.c
    src/pixman-fast-path.c
    src/pixman-filter.c
    src/pixman-general.c
    src/pixman-glyph.c
    src/pixman-gradient-walker.c
    src/pixman-image.c
    src/pixman-implementation.c
    src/pixman-linear-gradient.c
    src/pixman-matrix.c
    src/pixman-mips.c
    src/pixman-mips-dspr2.c
    src/pixman-mmx.c
    src/pixman-noop.c
    src/pixman-ppc.c
    src/pixman-radial-gradient.c
    src/pixman-region16.c
    src/pixman-region32.c
    src/pixman-solid-fill.c
    src/pixman-sse2.c
    src/pixman-ssse3.c
    src/pixman-timer.c
    src/pixman-trap.c
    src/pixman-utils.c
    src/pixman-x86.c)

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC src)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4244 /wd4146 /wd4305 /wd4267 /wd4311 /wd4996 /wd4090)
endif()
