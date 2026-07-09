#pragma once
#include <string>
#include <vector>

class AudioDecoder {
public:
    // 파일 경로를 주입하면 44100Hz Mono Float PCM 데이터 배열을 반환
    static std::vector<float> decodeToMonoFloatPCM(const std::string& filePath);

private:
    static std::vector<float> decodeWav(const std::string& filePath);
    static std::vector<float> decodeMp3(const std::string& filePath);

    // 타깃 샘플레이트(44100Hz)가 아닐 경우 선형 보간으로 강제 리샘플링
    static std::vector<float> resample(const std::vector<float>& input, unsigned int sourceRate, unsigned int targetRate);
};
