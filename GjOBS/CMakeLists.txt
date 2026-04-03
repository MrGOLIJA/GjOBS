cmake_minimum_required(VERSION 3.16)
project(GjOBS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/ffmpeg)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Multimedia)

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Multimedia
)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/outputdevice.cpp
    src/screenrecorder.cpp
    src/ffmpegManager.cpp
    src/settings.cpp
)

set(PROJECT_HEADERS
    hdr/ffmpegManager.h
    hdr/outputdevice.h
    hdr/screenrecorder.h
    hdr/settings.h
)

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        hdr
        ${FFMPEG_DIR}/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${FFMPEG_DIR}/lib
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Multimedia
    PRIVATE
        avformat
        avcodec
        swscale
        swresample
        avutil
)