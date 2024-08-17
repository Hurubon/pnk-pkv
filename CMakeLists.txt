cmake_minimum_required(VERSION 3.5)
project(pnk-pkv VERSION 0.0.1 LANGUAGES C)

add_executable(pkv main.c)

target_compile_features(pkv PRIVATE c_std_11)

target_compile_options(pkv PRIVATE -Ofast -g)

target_include_directories(pkv PRIVATE lib/ffmpeg/include)
target_include_directories(pkv PRIVATE inc)
target_include_directories(pkv PRIVATE lib)

target_link_directories(pkv PRIVATE lib/ffmpeg/lib)
target_link_libraries  (pkv PRIVATE avformat avcodec avutil swscale)

add_executable(scratch scratchpad.c)
