# 🔎 이게모지 (Emoji Semantic Search)

> **팀 9조**의 이모지 시맨틱 검색 프로젝트입니다.  
> 사용자가 입력한 단어의 의미를 파악하여 가장 적절한 이모지를 추천해줍니다.

## 🔗 배포 링크 (Demo)
웹 브라우저에서 아래 주소로 접속하여 바로 실행해 보실 수 있습니다.

👉 **https://areum0505.github.io/SearchImoji_Wasm/**

---

## 👥 팀 소개 (Team 9)

| 이름 | 직책 | 역할 |
|:---:|:---:|:---|
| **김아름** | 팀장 | **Main Developer** |
| **박건우** | 팀원 | **Main Developer**  |
| **이은지** | 팀원 | **Sub Developer & Design**  |

---

## 🚀 실행 방법 (How to Use)

별도의 설치 없이 웹 링크를 통해 사용할 수 있습니다.

1. [프로젝트 링크](https://areum0505.github.io/SearchImoji_Wasm/)에 접속합니다.
2. 입력창에 **검색하고 싶은 단어 및 문장**를 입력합니다. 
3. **`비교 실행`** 버튼을 클릭합니다.
4. 의미상 가장 유사한 **관련 이모지**가 하단에 출력되는 것을 확인합니다.

### 빌드 명령어
```
emcc emoji_search.cpp -o emoji_search.js -msimd128 -O3 -s "EXPORTED_FUNCTIONS=['_search_emojis', '_free_result_memory', '_malloc', '_free']" -s "EXPORTED_RUNTIME_METHODS=['cwrap', 'ccall', 'getValue', 'UTF8ToString', 'setValue']" -s MODULARIZE=0 -s ENVIRONMENT='web' -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -I.
```
---

## ⚡ 기술적 특징 (가산점 항목)

본 프로젝트는 성능 향상을 위해 다음과 같은 최적화 기술을 적용하였습니다.

* **128-bit SIMD 최적화 활성화:** Emscripten 빌드 시 SIMD(Single Instruction, Multiple Data) 옵션을 활성화하여, 작업 속도를 크게 향상시켰습니다.

---

## 🛠️ 개발 과정 및 회고 (Troubleshooting)

### ❗ 어려웠던 점
* **윈도우(Windows) 환경 설정 이슈:** 개발 초기, 팀원의 윈도우 PC에서 Wasm 환경설정하는 과정에서 잦은 충돌과 오류가 발생했습니다.

### ✅ 해결 방법
* **재설치 및 환경 통일:** 기존에 설치된 파이썬(Python)을 완전히 제거한 후 **재설치(Clean Install)**하여 경로 문제를 해결했습니다.

---

## 📊 Latency 측정 테이블

C++와 JavaScript의 성능 비교 결과입니다.

### 성능 비교

| Process | C++ | JS |
| :--- | :--- | :--- |
| **유사도 계산** | 3.9 ms | 5.6 ms |
| **결과 정렬** | 0.2 ms | 0.5 ms |
| **총 시간** | 4.1 ms | 6.1 ms |

* C++ 평균 소요 시간: 3.22 ms
* JS 평균 소요 시간: 6.23 ms

### 측정 환경
* OS/Browser: Windows 11 / Chrome 142
* CPU: Intel Core Ultra 5 225H
* Memory (RAM): 16 GB