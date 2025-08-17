cmake_minimum_required(VERSION 3.10)
project(pnk-pkv VERSION 0.2 LANGUAGES C)

add_executable(pkv main.c)

target_compile_features(pkv PRIVATE c_std_23)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(pkv PRIVATE
        -pedantic
        -Wall
        -Wextra
        -Werror
        -Wpedantic
        # -Wconversion (too many errors from FFmpeg)
        -Wshadow
        -Wstrict-prototypes
        -Wformat=2
        -Wundef
        -Wcast-qual
        -Wpointer-arith
        -Wwrite-strings
        -Wswitch-enum)
endif ()

find_package(PkgConfig REQUIRED)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWSCALE  REQUIRED libswscale)

target_include_directories(pkv PRIVATE
    lib
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS})

target_link_directories(pkv PRIVATE
    ${AVFORMAT_LIBRARY_DIRS}
    ${AVCODEC_LIBRARY_DIRS}
    ${AVUTIL_LIBRARY_DIRS}
    ${SWSCALE_LIBRARY_DIRS})

target_link_libraries(pkv PRIVATE
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES})
