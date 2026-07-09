#pragma once
#include <RtAudio.h>
#include <memory>

class RingBuffer;
class AudioVisualizer;

class AudioCapture {
public:
    explicit AudioCapture(std::shared_ptr<RingBuffer> ringBuffer);
    ~AudioCapture();

    bool start(unsigned int sampleRate = 44100, unsigned int bufferFrames = 512);

    void stop();

    bool isRecording() const { return m_audio.isStreamOpen(); }

    void setVisualizer(AudioVisualizer* visualizer) { m_visualizer = visualizer; }

private:
    static int audioCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                             double streamTime, RtAudioStreamStatus status, void* userData);

    RtAudio m_audio;
    std::shared_ptr<RingBuffer> m_ringBuffer;
    AudioVisualizer* m_visualizer = nullptr;
};
