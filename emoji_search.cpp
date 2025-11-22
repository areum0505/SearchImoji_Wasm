#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>

#include "emoji.hpp"   // EMBEDDINGS 정의 포함

constexpr int EMBEDDING_DIM = 768;
const int NUM_EMBEDDINGS = sizeof(EMBEDDINGS) / sizeof(EMBEDDINGS[0]);

struct SearchResult {
    int index;
    double score;
};

// -------------------- 수학 계산 --------------------
double calculateMagnitude(const double* vec) {
    double sum = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i)
        sum += vec[i] * vec[i];
    return std::sqrt(sum);
}

double cosineSimilarity(const double* dbVec, const double* queryVec) {
    double dotProduct = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i)
        dotProduct += dbVec[i] * queryVec[i];

    double magA = calculateMagnitude(dbVec);
    double magB = calculateMagnitude(queryVec);

    if (magA == 0.0 || magB == 0.0)
        return 0.0;
    return dotProduct / (magA * magB);
}

// -------------------- WASM 함수 --------------------
extern "C" {

    /**
     * @brief 768차원 쿼리 벡터를 받아 DB와 유사도 비교 후
     * 상위 5개를 JSON 문자열로 반환
     *
     * 반환 예:
     * {
     *   "results": [
     *      { "index": 120, "score": 0.8123 },
     *      ...
     *   ]
     * }
     */
    const char* search_emojis(const double* query_vector) {
        if (!query_vector) {
            const char* err = "{\"error\":\"Query vector is NULL\"}";
            char* alloc = new char[strlen(err) + 1];
            strcpy(alloc, err);
            return alloc;
        }

        if (NUM_EMBEDDINGS == 0) {
            const char* err = "{\"error\":\"Emoji database is empty\"}";
            char* alloc = new char[strlen(err) + 1];
            strcpy(alloc, err);
            return alloc;
        }

        std::vector<SearchResult> results;
        results.reserve(NUM_EMBEDDINGS);

        for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
            double score = cosineSimilarity(EMBEDDINGS[i], query_vector);
            results.push_back({ i, score });
        }

        std::sort(results.begin(), results.end(),
            [](const SearchResult& a, const SearchResult& b) {
                return a.score > b.score;
            });

        std::stringstream ss;
        ss << "{ \"results\": [";

        int limit = std::min(5, NUM_EMBEDDINGS);
        for (int i = 0; i < limit; ++i) {
            if (i > 0) ss << ",";
            ss << "{"
               << "\"index\":" << results[i].index << ","
               << "\"score\":" << std::fixed << std::setprecision(4) << results[i].score
               << "}";
        }
        ss << "] }";

        std::string json_str = ss.str();
        char* out = new char[json_str.size() + 1];
        memcpy(out, json_str.c_str(), json_str.size() + 1);
        return out;
    }

    /**
     * @brief WASM에서 생성한 JSON 문자열 메모리 해제
     */
    void free_result_memory(const char* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
}
