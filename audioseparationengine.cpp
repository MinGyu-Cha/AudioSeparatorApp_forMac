#include "AudioSeparationEngine.h"
#include <iostream>
#include <array>
#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include <QString>

AudioSeparationEngine::AudioSeparationEngine()
    : m_env(ORT_LOGGING_LEVEL_WARNING, "AudioSeparationEngine") {}

bool AudioSeparationEngine::initEngine(const std::string& modelPath) {
    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(4);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        std::cout << "[AI Engine] ONNX 모델 바인딩 시작: " << modelPath << std::endl;
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), sessionOptions);
        std::cout << "[AI Engine] AI 모델 로드 및 기동 성공!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[AI Engine] 초기화 실패 (파일 패스를 다시 확인하세요): " << e.what() << std::endl;
        return false;
    }
    return true;
}

void AudioSeparationEngine::processAudioFile(const std::vector<float>& audioData) {
    if (audioData.empty() || !m_session) return;

    QString appDir = QCoreApplication::applicationDirPath();
    QDir baseDir(appDir);
    baseDir.cdUp(); // mach_o_64bit_Debug 탈출
    baseDir.cdUp(); // arm_darwin_generic... 탈출

    std::string vocalPath = baseDir.filePath("vocal_output.wav").toStdString();
    std::string bgmPath = baseDir.filePath("accompaniment_output.wav").toStdString();

    std::cout << "[AI Engine] 보컬 저장 경로: " << vocalPath << std::endl;
    std::cout << "[AI Engine] 반주 저장 경로: " << bgmPath << std::endl;

    WavFileWriter vocalWriter;
    WavFileWriter bgmWriter;

    // 동적으로 계산된 프로젝트 루트 경로에 파일 생성 (44100Hz, 2채널 스테레오)
    vocalWriter.open(vocalPath, 44100, 2);
    bgmWriter.open(bgmPath, 44100, 2);

    Ort::AllocatorWithDefaultOptions allocator;

    size_t numInputNodes = m_session->GetInputCount();
    size_t numOutputNodes = m_session->GetOutputCount();

    std::cout << "[AI Engine] 모델 분석 완료 -> 입력 노드 수: " << numInputNodes
              << " / 출력 노드 수: " << numOutputNodes << std::endl;

    std::vector<std::string> inputNodeNames;
    std::vector<const char*> inputNames;
    for (size_t i = 0; i < numInputNodes; ++i) {
        auto namePtr = m_session->GetInputNameAllocated(i, allocator);
        inputNodeNames.push_back(namePtr.get());
        inputNames.push_back(inputNodeNames.back().c_str());
        std::cout << "  -> 입력 [" << i << "]: " << inputNodeNames.back() << std::endl;
    }

    std::vector<std::string> outputNodeNames;
    std::vector<const char*> outputNames;
    for (size_t i = 0; i < numOutputNodes; ++i) {
        auto namePtr = m_session->GetOutputNameAllocated(i, allocator);
        outputNodeNames.push_back(namePtr.get());
        outputNames.push_back(outputNodeNames.back().c_str());
        std::cout << "  -> 출력 [" << i << "]: " << outputNodeNames.back() << std::endl;
    }

    std::cout << "[AI Engine] 음향 신호 분리 매트릭스 연산 돌입..." << std::endl;
    // =================================================================

    size_t stereoChunkSize = m_chunkSize * 2;
    std::vector<float> chunkInput(stereoChunkSize, 0.0f);

    size_t totalSamples = audioData.size();
    size_t offset = 0;

    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    while (offset < totalSamples) {
        size_t copySize = std::min(stereoChunkSize, totalSamples - offset);
        std::fill(chunkInput.begin(), chunkInput.end(), 0.0f);
        std::copy(audioData.begin() + offset, audioData.begin() + offset + copySize, chunkInput.begin());

        std::array<int64_t, 3> inputShape = {1, 2, static_cast<int64_t>(m_chunkSize)};

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, chunkInput.data(), stereoChunkSize, inputShape.data(), inputShape.size()
            );

        try {
            auto outputTensors = m_session->Run(
                Ort::RunOptions{nullptr},
                inputNames.data(), &inputTensor, inputNames.size(),
                outputNames.data(), outputNames.size()
                );

            if (outputTensors.size() >= 2) {
                // 출력이 2개 분리형으로 나오는 경우 (0번 보컬, 1번 반주)
                float* pVocalData = outputTensors[0].GetTensorMutableData<float>();
                float* pBgmData = outputTensors[1].GetTensorMutableData<float>();

                vocalWriter.write(pVocalData, stereoChunkSize);
                bgmWriter.write(pBgmData, stereoChunkSize);
            }
            else if (outputTensors.size() == 1) {
                // 출력이 1개 묶음형으로 나오는 경우 (4차원 텐서 등)
                float* pAllData = outputTensors[0].GetTensorMutableData<float>();

                // 임시로 데이터를 쪼개서 분기해 둠 (구조 확인용)
                vocalWriter.write(pAllData, stereoChunkSize);
                bgmWriter.write(chunkInput.data(), stereoChunkSize);
            }

            std::cout << "[AI Worker] 연산 완료 -> 진행률: "
                      << static_cast<int>((static_cast<double>(offset) / totalSamples) * 100) << "%" << std::endl;
        }
        catch (const Ort::Exception& e) {
            std::cerr << "[ONNX 엔진 에러]: " << e.what() << std::endl;
            break;
        }

        offset += stereoChunkSize;
    }

    vocalWriter.close();
    bgmWriter.close();
    std::cout << "[AI Complete] 파이프라인 연산이 안전하게 종료되었습니다." << std::endl;
}
