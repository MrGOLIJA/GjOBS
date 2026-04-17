cmake_minimum_required(VERSION 3.16)
project(GjOBS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/ffmpeg)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Multimedia Qml Concurrent)

find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Multimedia
        Qml
        Concurrent
)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/outputdevice.cpp
    src/screenrecorder.cpp
    src/ffmpegManager.cpp
    src/settings.cpp
    src/AudioCodersSrc/AACAudioCoder.cpp
    src/VideoCodersSrc/H_264VideoCoder.cpp
    src/VideoCodersSrc/H_264_NVENC_VideoCoder.cpp

)

set(PROJECT_HEADERS
    hdr/ffmpegManager.h
    hdr/outputdevice.h
    hdr/screenrecorder.h
    hdr/settings.h
    hdr/AudioCoder.h
    hdr/VideoCoder.h
    hdr/Coder.h
    hdr/AudioCodersHdr/AACAudioCoder.h
    hdr/VideoCodersHdr/H_264VideoCoder.h
    hdr/VideoCodersHdr/H_264_NVENC_VideoCoder.h

)

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

qt_add_qml_module(${PROJECT_NAME}
    URI GjOBS
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/ToolsLayout.qml
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
        Qt::Qml
        Qt::Concurrent
    PRIVATE
        avformat
        avcodec
        swscale
        swresample
        avutil
        avfilter
        runtimeobject
)