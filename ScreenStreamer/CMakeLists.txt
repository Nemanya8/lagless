cmake_minimum_required(VERSION 3.15)
project(ScreenStreamer)

set(CMAKE_CXX_STANDARD 17)

# FFmpeg paths - adjust these to match your FFmpeg installation
set(FFMPEG_ROOT "C:/Users/Administrator/Desktop/ffmpeg-master-latest-win64-gpl-shared" CACHE PATH "Path to FFmpeg installation")

# Include directories
include_directories(
    ${FFMPEG_ROOT}/include
)

# Library directories
link_directories(
    ${FFMPEG_ROOT}/lib
)

add_executable(ScreenStreamer
    main.cpp
    CaptureManager.cpp
    CaptureManager.h
    FFmpegEncoder.cpp
    FFmpegEncoder.h
    UdpSender.cpp
    UdpSender.h
)

target_link_libraries(ScreenStreamer
    d3d11
    dxgi
    ws2_32
    avcodec
    avformat
    avutil
    swscale
)
