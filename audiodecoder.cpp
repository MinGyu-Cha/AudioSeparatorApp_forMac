#include "AudioDecoder.h"
#include <iostream>
#include <algorithm>

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"

std::vector<float> AudioDecoder::decodeToMonoFloatPCM(const std::string& filePath) {
    std::string ext = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "wav") {
        return decodeWav(filePath);
    } else if (ext == "mp3") {
        return decodeMp3(filePath);
    } else {
        std::cerr << "[Decoder] 지원하지 않는 파일 형식입니다: " << ext << std::endl;
        return {};
    }
}

std::vector<float> AudioDecoder::decodeWav(const std::string& filePath) {
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;

    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(
        filePath.c_str(), &channels, &sampleRate, &totalPCMFrameCount, nullptr);

    if (pSampleData == nullptr) return {};

    // 2채널(스테레오)을 강제 타겟팅
    // 모노 파일이면 2채널로 복사하고, 5.1채널이면 앞 2채널만 사용 (임시)
    size_t targetChannels = 2;
    std::vector<float> output(totalPCMFrameCount * targetChannels);

    for (drwav_uint64 i = 0; i < totalPCMFrameCount; ++i) {
        if (channels >= 2) {
            output[i * 2 + 0] = pSampleData[i * channels + 0]; // 좌(L)
            output[i * 2 + 1] = pSampleData[i * channels + 1]; // 우(R)
        } else {
            // 모노 파일인 경우 복사해서 채움
            output[i * 2 + 0] = pSampleData[i];
            output[i * 2 + 1] = pSampleData[i];
        }
    }

    drwav_free(pSampleData, nullptr);

    std::cout << "[Decoder] WAV 로드 성공. (44100Hz Stereo)" << std::endl;
    return output;
}

std::vector<float> AudioDecoder::decodeMp3(const std::string& filePath) {
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, filePath.c_str(), nullptr)) return {};

    std::vector<float> tempBuffer;
    float sample[2048];

    while (true) {
        drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(&mp3, 1024, sample);
        if (framesRead == 0) break;
        tempBuffer.insert(tempBuffer.end(), sample, sample + (framesRead * mp3.channels));
    }

    size_t totalFrames = tempBuffer.size() / mp3.channels;
    std::vector<float> output(totalFrames * 2);

    for (size_t i = 0; i < totalFrames; ++i) {
        if (mp3.channels >= 2) {
            output[i * 2 + 0] = tempBuffer[i * mp3.channels + 0];
            output[i * 2 + 1] = tempBuffer[i * mp3.channels + 1];
        } else {
            output[i * 2 + 0] = tempBuffer[i];
            output[i * 2 + 1] = tempBuffer[i];
        }
    }

    drmp3_uninit(&mp3);
    std::cout << "[Decoder] MP3 로드 성공. (44100Hz Stereo)" << std::endl;
    return output;
}

std::vector<float> AudioDecoder::resample(const std::vector<float>& input, unsigned int sourceRate, unsigned int targetRate) {
    if (input.empty() || sourceRate == targetRate) return input;

    double factor = static_cast<double>(targetRate) / sourceRate;
    size_t newSize = static_cast<size_t>(input.size() * factor);
    std::vector<float> output(newSize);

    // 선형보간 알고리즘 필요시 보강 필요
    for (size_t i = 0; i < newSize; ++i) {
        double srcIdx = i / factor;
        size_t idxLow = static_cast<size_t>(std::floor(srcIdx));
        size_t idxHigh = std::min(idxLow + 1, input.size() - 1);
        double weight = srcIdx - idxLow;

        output[i] = static_cast<float>((1.0 - weight) * input[idxLow] + weight * input[idxHigh]);
    }
    return output;
}
