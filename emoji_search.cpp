#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdio> // snprintf 사용을 위해 추가

// emoji.hpp에는 벡터 데이터만 포함되어 있음
#include "emoji.hpp" 

// [emoji.hpp에 정의되어야 하는 내용]
constexpr int EMBEDDING_DIM = 768;
const int NUM_EMBEDDINGS = sizeof(EMBEDDINGS) / sizeof(EMBEDDINGS[0]);

struct SearchResult {
    int index;
    double score;
};

// --- [1] 수학 연산 함수들 ---
double calculateMagnitude(const double* vec) {
    double sum = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i) sum += vec[i] * vec[i];
    return std::sqrt(sum);
}

double cosineSimilarity(const double* dbVec, const double* queryVec) {
    double dotProduct = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i) {
        dotProduct += dbVec[i] * queryVec[i];
    }
    double magA = calculateMagnitude(dbVec);
    double magB = calculateMagnitude(queryVec); 
    
    if (magA == 0.0 || magB == 0.0) return 0.0;
    return dotProduct / (magA * magB);
}

// --- [2] WASM으로 노출할 함수 정의 ---
extern "C" {
    
    /**
     * @brief 쿼리 벡터를 받아 유사도 검색을 수행하고 
     * 상위 5개 결과의 인덱스와 점수를 JSON 문자열로 반환합니다.
     * @param query_vector 768차원 임베딩 벡터 (double* 포인터)
     * @return 상위 5개 결과의 인덱스와 점수를 담은 JSON 문자열 (C-스타일 문자열)
     */
    const char* search_emojis(const double* query_vector) {
        if (!query_vector) {
            return "{\"error\":\"Query vector is NULL.\"}";
        }
        
        const int NUM_EMBEDDINGS = sizeof(EMBEDDINGS) / sizeof(EMBEDDINGS[0]);

        if (NUM_EMBEDDINGS == 0) {
            return "{\"error\":\"Emoji database is empty.\"}";
        }

        // 1. 유사도 계산
        std::vector<SearchResult> results;
        results.reserve(NUM_EMBEDDINGS);

        for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
            double score = cosineSimilarity(EMBEDDINGS[i], query_vector); 
            results.push_back({i, score});
        }

        // 2. 정렬 (높은 점수 순)
        std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
            return a.score > b.score;
        });

        // 3. 상위 5개 결과를 JSON 문자열로 변환 (인덱스 + 점수)
        std::stringstream ss;
        ss << "[";
        int limit = std::min(5, NUM_EMBEDDINGS);

        for (int i = 0; i < limit; ++i) {
            int idx = results[i].index;
            
            if (i > 0) ss << ",";
            
            // JSON 문자열 생성: 인덱스와 점수만 포함
            // JavaScript가 이 인덱스를 사용하여 실제 이모지와 설명을 찾습니다.
            ss << "{\"index\":" << idx
               << ",\"score\":" << std::fixed << std::setprecision(4) << results[i].score << "}";
        }
        ss << "]";

        // 4. 결과 문자열을 힙에 할당하여 WASM 메모리 공간에 반환
        std::string result_str = ss.str();
        char* c_str = new char[result_str.length() + 1];
        std::memcpy(c_str, result_str.c_str(), result_str.length() + 1);
        
        return c_str;
    }

    /**
     * @brief search_emojis에서 힙에 할당된 메모리를 해제하기 위한 함수
     */
    void free_result_memory(char* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
}