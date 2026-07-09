#pragma once
#include <string>
#include <vector>
#include <memory>
#include <onnxruntime_cxx_api.h>
#include "WavFileWriter.h"

class AudioSeparationEngine {
public:
    AudioSeparationEngine();
    ~AudioSeparationEngine() = default;

    bool initEngine(const std::string& modelPath);

    // 파일 기반 인터페이스: 오디오 데이터를 집어넣으면 분리 후 저장
    void processAudioFile(const std::vector<float>& audioData);

private:
    Ort::Env m_env;
    std::unique_ptr<Ort::Session> m_session{nullptr};

    const size_t m_chunkSize = 343980; // 46080에서 모델 요구 크기로 상향
};
