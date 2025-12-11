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

---

## 동작 흐름
검색 버튼을 클릭하면 다음과 같은 과정이 순식간에 일어납니다.

1.  **[JS]** 사용자가 입력한 텍스트를 가져옵니다.
2.  **[JS]** `Transformers.js` AI 모델이 텍스트의 의미를 분석하여 768차원의 **임베딩 벡터**로 변환합니다.
3.  **[JS → C++]** 생성된 벡터를 WebAssembly로 컴파일된 C++ 함수(`search_emojis`)에 전달합니다.
4.  **[C++]** search_emojis 함수는 입력 벡터와 1300개 이상의 모든 이모지 벡터 간의 **코사인 유사도**를 계산합니다.
5.  **[C++]** 가장 유사한 상위 5개의 이모지를 효율적으로 찾아냅니다.
6.  **[C++ → JS]** 최종 결과를 JSON 문자열 형태로 JavaScript에 반환합니다.
7.  **[JS]** 전달받은 결과를 사용자가 보기 쉽게 UI에 렌더링합니다.

---

## 핵심 기능

프로젝트의 핵심 기능은 **1) JavaScript에서 사용자 입력을 벡터로 변환**하고, **2) C++(Wasm)에서 이 벡터를 사용해 유사도 높은 이모지를 검색**하는 두 단계로 나뉩니다.

### 1. 텍스트 임베딩 (JavaScript) - 검색 버튼 클릭 시

사용자가 검색 버튼을 클릭하면, 입력된 텍스트는 JavaScript 코드에 의해 의미론적 벡터(임베딩)로 변환됩니다.

```javascript
// '비교 실행' 버튼 클릭 이벤트 리스너 내부
document.getElementById('runBtn').addEventListener('click', async () => {
    // 1. 사용자 입력 텍스트를 가져옵니다.
    const text = document.getElementById('userInput').value.trim();
    if (!text) return;

    // 2. 입력 텍스트의 언어에 따라 적절한 임베딩 모델(extractor)을 가져옵니다.
    const extractor = await window.getExtractorForText(text);

    // 3. 텍스트를 고차원 벡터(임베딩)로 변환합니다.
    const output = await extractor(text, { pooling: 'mean', normalize: true });
    const vector = Array.from(output.data);

    // ... (이후 임베딩 벡터를 WASM/JS 검색 함수에 전달)
});
```
- **간단한 설명**: `transformers.js` 라이브러리를 통해 사용자의 텍스트 입력을 768차원의 벡터로 변환하여, 이후의 유사도 계산 단계에 전달합니다.

### 2. 유사도 검색 및 Top-N 추출 (C++)

JavaScript로부터 전달받은 쿼리 벡터를 사용하여, 미리 정의된 전체 이모지 임베딩과 비교해 가장 유사한 상위 5개의 결과를 효율적으로 찾아냅니다.

```cpp
const char* search_emojis(const double* query_vector) {
    // 1. 데이터베이스 벡터 크기 캐시에서 로드
    static std::vector<double> db_magnitudes;
    /* ... */

    // 2. 쿼리 벡터 크기 계산
    double query_mag = calculateMagnitude(query_vector);

    // 3. 최소 힙(Min-Heap)으로 Top-N 결과를 담을 우선순위 큐 선언
    std::priority_queue<SearchResult, std::vector<SearchResult>, std::greater<SearchResult>> top_results;

    for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
        // 4. 코사인 유사도 계산
        double score = cosineSimilarity(EMBEDDINGS[i], db_magnitudes[i], query_vector, query_mag);

        // 5. 최소 힙을 이용해 상위 5개 결과 유지
        /* ... */
    }
    /* ... 결과 정렬 및 JSON 변환 ... */
}
```
- **간단한 설명**: C++로 구현되어 WebAssembly로 컴파일된 이 함수는, 최적화된 로직을 통해 1300개가 넘는 이모지와의 유사도를 빠르게 계산하고 가장 관련성 높은 결과를 반환합니다.

### 3. 코사인 유사도 계산 (C++)

두 벡터가 얼마나 유사한 방향을 가리키는지를 측정하는 함수입니다. 1에 가까울수록 유사도가 높습니다.

```cpp
double cosineSimilarity(const double* dbVec, double dbMag, const double* queryVec, double queryMag) {
    double dotProduct = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i)
        dotProduct += dbVec[i] * queryVec[i];

    if (dbMag == 0.0 || queryMag == 0.0)
        return 0.0;
    return dotProduct / (dbMag * queryMag);
}
```
- **간단한 설명**: 두 벡터의 내적(dot product)을 각 벡터의 크기(magnitude)의 곱으로 나누어 유사도를 계산합니다.

## ⚡ 기술적 특징 (가산점 항목)

본 프로젝트는 성능 향상을 위해 다음과 같은 최적화 기술을 적용하였습니다.

### C++/JavaScript 알고리즘 최적화
- **벡터 크기 사전 캐싱**: 이모지 임베딩 데이터는 변하지 않으므로, 유사도 계산에 필요한 각 벡터의 크기를 계산하는 과정은 비용이 큽니다. **WASM 모듈이 로드되는 시점에 미리 계산**하여 캐시에 저장합니다. 이를 통해 비용이 큰 `sqrt` 연산을 수천 번 반복하는 것을 방지하였고, 첫 검색 시 발생하던 지연 시간을 제거하여 사용자는 C++(WASM)의 빠른 성능을 첫 검색부터 경험할 수 있습니다.
- **최소 힙(Min-Heap) 사용**: 상위 5개 결과를 찾기 위해 C++에서는 `std::priority_queue`를, JavaScript에서는 `sort`를 이용한 효율적인 배열 관리를 통해 전체 결과를 매번 정렬할 필요 없이 O(log N) 또는 그에 준하는 복잡도로 상위 N개를 효율적으로 유지합니다.

### 128-bit SIMD 최적화 활성화
- Emscripten 빌드 시 SIMD(Single Instruction, Multiple Data) 옵션을 활성화하여, 벡터 연산이 많은 코사인 유사도 계산과 같은 작업의 속도를 크게 향상시켰습니다.

#### 빌드 명령어
```
emcc emoji_search.cpp -o emoji_search.js -msimd128 -O3 -s "EXPORTED_FUNCTIONS=['_search_emojis', '_free_result_memory', '_precalculate_db_magnitudes', '_malloc', '_free']" -s "EXPORTED_RUNTIME_METHODS=['cwrap', 'ccall', 'getValue', 'UTF8ToString', 'setValue']" -s MODULARIZE=0 -s ENVIRONMENT='web' -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -I.
```

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
<details>
<summary>최적화 이전</summary>

<br>

| Process | C++ | JS |
| :--- | :--- | :--- |
| **유사도 계산** | 3.9 ms | 5.6 ms |
| **결과 정렬** | 0.2 ms | 0.5 ms |
| **총 시간** | 4.1 ms | 6.1 ms |

* C++ 평균 소요 시간: 3.22 ms
* JS 평균 소요 시간: 6.23 ms
</details>

---

| Process | C++ | JS |
| :--- | :--- | :--- |
| **유사도 계산** | 1.5 ms | 2.7 ms |
| **총 시간** | 1.5 ms | 2.7 ms |

* C++ 평균 소요 시간: 1.78 ms
* JS 평균 소요 시간: 2.65 ms


### 측정 환경
* OS/Browser: Windows 11 / Chrome 142
* CPU: Intel Core Ultra 5 225H
* Memory (RAM): 16 GB