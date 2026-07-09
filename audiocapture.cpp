#include "AudioCapture.h"
#include "RingBuffer.h"
#include "audiovisualizer.h"
#include <iostream>

AudioCapture::AudioCapture(std::shared_ptr<RingBuffer> ringBuffer)
    : m_ringBuffer(ringBuffer) {}

AudioCapture::~AudioCapture() {
    stop();
}

bool AudioCapture::start(unsigned int sampleRate, unsigned int bufferFrames) {
    if (m_audio.getDeviceCount() < 1) {
        std::cerr << "[AudioCapture] 이용 가능한 오디오 장치가 없습니다." << std::endl;
        return false;
    }

    RtAudio::StreamParameters inputParams;
    inputParams.deviceId = m_audio.getDefaultInputDevice();
    inputParams.nChannels = 1; // Mono
    inputParams.firstChannel = 0;

    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_MINIMIZE_LATENCY;

    try {
        // RTAUDIO_FLOAT32
        m_audio.openStream(nullptr, &inputParams, RTAUDIO_FLOAT32, sampleRate,
                           &bufferFrames, &audioCallback, this, &options);

        m_audio.startStream();
        std::cout << "[AudioCapture] 오디오 캡처 스트림 시작 성공 ("
                  << sampleRate << "Hz, Buffer: " << bufferFrames << ")" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[AudioCapture] 스트림 종료 중 에러: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void AudioCapture::stop() {
    try {
        if (m_audio.isStreamRunning()) {
            m_audio.stopStream();
        }
        if (m_audio.isStreamOpen()) {
            m_audio.closeStream();
            std::cout << "[AudioCapture] 오디오 캡처 스트림 종료" << std::endl;
        }
    }
    catch (const std::exception&  e) {
        std::cerr << "[AudioCapture] 스트림 종료 중 에러: " << e.what() << std::endl;
    }
}

int AudioCapture::audioCallback(void* /*outputBuffer*/, void* inputBuffer, unsigned int nBufferFrames,
                                double /*streamTime*/, RtAudioStreamStatus status, void* userData) {
    if (status || inputBuffer == nullptr || nBufferFrames == 0) return 0;

    auto* app = static_cast<AudioCapture*>(userData);
    if (app) {
        const auto* pcmStream = static_cast<const float*>(inputBuffer);

        if (app->m_ringBuffer) {
            app->m_ringBuffer->write(pcmStream, nBufferFrames);
        }

        if (app->m_visualizer) {
            app->m_visualizer->pushAudioData(pcmStream, nBufferFrames);
            app->m_visualizer->update();
        }
    }

    return 0;
}
