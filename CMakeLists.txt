cmake_minimum_required(VERSION 3.16)
project(AudioSeparatorApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# M3 Homebrew 경로 지정
set(HOMEBREW_PREFIX "/opt/homebrew")
list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX})

# Qt6 필수 컴포넌트 불러오기
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ----------------------------------------------------------------
# 발견된 물리 경로를 기반으로 정적 힌트 지정
# ----------------------------------------------------------------
set(RTAUDIO_INCLUDE_DIR "${HOMEBREW_PREFIX}/include")
set(RTAUDIO_LIBRARY "${HOMEBREW_PREFIX}/lib/librtaudio.dylib")

message(STATUS "-> Target RtAudio Include: ${RTAUDIO_INCLUDE_DIR}")
message(STATUS "-> Target RtAudio Library: ${RTAUDIO_LIBRARY}")

# =================================================================
# 🚀 [ONNX Runtime 검색] M3 맥북 Homebrew 경로 타겟팅
# =================================================================
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h HINTS /opt/homebrew/include/onnxruntime)
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime HINTS /opt/homebrew/lib)
# ----------------------------------------------------------------

# 실행 파일 생성 및 소스 지정
add_executable(AudioSeparatorApp
    main.cpp
    mainwindow.h
    mainwindow.cpp
    RingBuffer.h
    RingBuffer.cpp
    AudioCapture.h
    AudioCapture.cpp
    audiovisualizer.h
    audiovisualizer.cpp
    wavfilewriter.h
    wavfilewriter.cpp
    dr_libs/dr_wav.h
    dr_libs/dr_mp3.h
    audiodecoder.h
    audiodecoder.cpp
    audioseparationengine.h audioseparationengine.cpp
)

# Qt6 라이브러리 링크 및 RtAudio 경로 연결
target_include_directories(AudioSeparatorApp PRIVATE
    ${RTAUDIO_INCLUDE_DIR}
    "${RTAUDIO_INCLUDE_DIR}/rtaudio"
    ${ONNXRUNTIME_INCLUDE_DIR}  # ONNX 헤더 추가
)
target_link_libraries(AudioSeparatorApp PRIVATE
    Qt6::Widgets
    ${RTAUDIO_LIBRARY}
    ${ONNXRUNTIME_LIBRARY}      # ONNX 바이너리 링크 추가
)