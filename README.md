# AudioSaparatorApp_forMac

**Qt6**와 **ONNX Runtime**을 기반으로 동작하도록 설계된 
macOS(Apple Silicon)용 인공지능 음원 분리(보컬/반주) 애플리케이션입니다.

---



## 주요 기능
* **크로스플랫폼 오디오 디코딩:** `dr_libs`를 사용하여 MP3 및 WAV 파일을 표준 규격인 44100Hz 32-bit Float Stereo PCM 배열로 읽어옵니다.
* **하드웨어 가속 추론:** **ONNX Runtime C++ API**가 내장되었습니다.
* **동적 노드 매칭 :** 런타임에 AI 모델의 메타데이터를 직접 쿼리하여 입력/출력 텐서 이름을 자동으로 바인딩합니다.



---



## 시스템 동작 원리 및 데이터 흐름

본 애플리케이션은 대용량 오디오 스트림 데이터가 딥러닝 행렬 연산 구역으로 통과할 수 있도록 고안된 파이프라인으로 동작합니다.

```text
[ 입력 오디오 파일 (MP3 / WAV) ]
               │
               ▼ (dr_libs 가동: Stereo, 44100Hz, F32 PCM 추출)
[ 2채널 인터리빙 Float 데이터 배열 ]
               │
               ▼ (윈도우 슬라이싱: 모델 요구 청크 크기인 343,980 샘플 단위 정렬)
[ ONNX Runtime Session (htdemucs.onnx) ] ◄── 런타임 동적 노드 이름 쿼리 및 매핑
               │
               ├──► [출력 텐서 0번: Vocal] ──► WavFileWriter를 통해 vocal_output.wav 생성
               └──► [출력 텐서 1번: BGM]   ──► WavFileWriter를 통해 accompaniment_output.wav 생성
```


1. **디코딩 단계:** 사용자가 선택한 오디오 파일을 압축 해제하여 표준 좌/우(L/R) 스테레오 오디오 알맹이로 변환합니다.
2. **청크 슬라이싱 단계:** 연속된 오디오 데이터를 `htdemucs` 인공지능 모델이 요구하는
   다차원 텐서 형상($1 \times 2 \times 343980$, 약 7.8초 분량)으로 조각내어 정렬합니다.
3. **AI 추론 단계:** 정렬된 텐서를 ONNX 모델에 주입하고, M3 프로세서의 고성능 코어 할당 효율을 높여 병렬 매트릭스 연산을 수행합니다.
4. **스트리밍 라이터 단계:** 분리되어 튀어나온 Float 버퍼 데이터를 파일에 순차적으로 기록하여 메모리 누수를 방지하고 대용량 파일도 안정적으로 처리합니다.



---



## 개발 환경 및 의존성

* **운영체제(OS):** macOS (Apple Silicon M3 이상 아키텍처 권장)
* **프레임워크:** Qt 6.x (Core, Widgets)
* **추론 엔진:** ONNX Runtime (C/C++ API)
* **빌드 시스템:** CMake (최소 버전 3.16 이상)

### Homebrew를 통한 필수 의존성 설치 커맨드
```bash
brew install qt
brew install onnxruntime
```



---



## 사용 방법
AI 모델 및 테스트 음원 배치:
분리하고 싶은 음악 파일을 test.mp3로 이름을 변경하여 최상위 루트 폴더에 배치하고 앱을 실행.

```text
AudioSaparatorApp_forMac/
├── demucs_model.onnx   <-- AI 모델 파일 배치
├── test.mp3            <-- 테스트할 음악 파일 배치
├── main.cpp
└── ...
```

## 최종 산출물 안내 (Artifacts)
연산이 성공적으로 완료되면, 하드웨어 가속 행렬 연산을 거쳐 프로젝트 최상위 루트 폴더에 스튜디오 규격의 32-bit Float 스테레오 .wav 파일 2개가 생성됩니다.

[vocal_output.wav 🎤](https://raw.githubusercontent.com/MinGyu-Cha/AudioSeparatorApp_forMac/main/vocal_output.wav): 원곡에서 악기 소리가 지워지고, 가수의 목소리/숨소리만 추출된 보컬 음원 파일입니다.

🎸 accompaniment_output.wav: 원곡에서 가수의 목소리 주파수만 지워지고, 드럼/베이스/기타 등 세션 연산만 남은 MR(반주) 파일입니다.



# 정상 구동 시 터미널 로그 예시
```text
[Decoder] MP3 로드 성공. (44100Hz Stereo)
[AI Engine] ONNX 모델 바인딩 시작: .../demucs_model.onnx
[AI Engine] AI 모델 로드 및 기동 성공!
[AI Engine] 모델 분석 완료 -> 입력 노드 수: 1 / 출력 노드 수: 2
  -> 입력 [0]: mix
  -> 출력 [0]: vocal
  -> 출력 [1]: no_vocal
[AI Engine] 음향 신호 분리 매트릭스 연산 돌입...
[AI Worker] 연산 완료 -> 진행률: 100%
[AI Complete] 파이프라인 연산이 안전하게 종료되었습니다.
```
