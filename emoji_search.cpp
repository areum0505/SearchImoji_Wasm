#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <chrono>
#include <queue>

#include "emoji.hpp"   // EMBEDDINGS 정의 포함

constexpr int EMBEDDING_DIM = 768;
const int NUM_EMBEDDINGS = sizeof(EMBEDDINGS) / sizeof(EMBEDDINGS[0]);

struct SearchResult {
    int index;
    double score;

    // priority_queue를 최소 힙(min-heap)으로 사용하기 위한 비교 연산자입니다.
    bool operator>(const SearchResult& other) const {
        return score > other.score;
    }
};

// -------------------- 수학 계산 --------------------
/**
 * @brief 벡터의 크기(magnitude)를 계산합니다.
 * @param vec 크기를 계산할 벡터 (double 배열)
 * @return 벡터의 크기 (double)
 */
double calculateMagnitude(const double* vec) {
    double sum = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i)
        sum += vec[i] * vec[i];
    return std::sqrt(sum);
}

/**
 * @brief 두 벡터 간의 코사인 유사도를 계산합니다.
 *        미리 계산된 벡터 크기를 인자로 받아 중복 계산을 피합니다.
 * @param dbVec 데이터베이스 벡터
 * @param dbMag dbVec의 미리 계산된 크기
 * @param queryVec 쿼리 벡터
 * @param queryMag queryVec의 미리 계산된 크기
 * @return 두 벡터 간의 코사인 유사도
 */
double cosineSimilarity(const double* dbVec, double dbMag, const double* queryVec, double queryMag) {
    double dotProduct = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i)
        dotProduct += dbVec[i] * queryVec[i];

    if (dbMag == 0.0 || queryMag == 0.0)
        return 0.0;
    return dotProduct / (dbMag * queryMag);
}

// -------------------- WASM 함수 --------------------
constexpr int TOP_N = 5;

extern "C" {

/**
 * @brief 쿼리 벡터와 전체 임베딩 데이터를 사용하여 유사도 검색을 수행합니다.
 *        가장 유사한 상위 N개의 이모지를 찾아 JSON 형식의 문자열로 반환합니다.
 * @param query_vector 사용자가 입력한 텍스트의 임베딩 벡터 (double 배열)
 * @return JSON 형식의 문자열. 예: `{"time_ms": 12.345, "results": [{"index": 0, "score": 0.98}, ...]}`
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

        // 전체 처리 시간 측정 시작
        auto start_time = std::chrono::high_resolution_clock::now();

        // 데이터베이스 임베딩의 크기를 미리 계산하여 캐시합니다.
        static std::vector<double> db_magnitudes; // static 변수 사용
        if (db_magnitudes.empty()) {
            db_magnitudes.reserve(NUM_EMBEDDINGS);
            for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
                db_magnitudes.push_back(calculateMagnitude(EMBEDDINGS[i]));
            }
        }

        // 쿼리 벡터의 크기 계산
        double query_mag = calculateMagnitude(query_vector);

        // 상위 N개의 결과를 효율적으로 관리하기 위해 최소 힙(min-heap)으로 구현된 priority_queue를 사용합니다.
        // 점수가 가장 낮은 항목이 항상 top()에 위치하게 됩니다.
        std::priority_queue<SearchResult, std::vector<SearchResult>, std::greater<SearchResult>> top_results;

        for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
            // 코사인 유사도를 계산합니다.
            double score = cosineSimilarity(EMBEDDINGS[i], db_magnitudes[i], query_vector, query_mag);

            // 힙의 크기가 TOP_N보다 작으면, 새 결과를 추가합니다.
            if (top_results.size() < TOP_N) {
                top_results.push({i, score});
            } 
            // 힙이 꽉 찼고, 새 점수가 힙에서 가장 작은 점수(top)보다 크면,
            // 가장 작은 것을 제거하고 새 결과를 추가합니다.
            else if (score > top_results.top().score) {
                top_results.pop();
                top_results.push({i, score});
            }
        }

        // 최종 결과 반환을 위해 최소 힙에서 모든 원소를 꺼내 벡터에 담습니다.
        std::vector<SearchResult> final_results;
        final_results.reserve(top_results.size());
        while (!top_results.empty()) {
            final_results.push_back(top_results.top());
            top_results.pop();
        }
        // 클라이언트에서는 점수가 높은 순서(내림차순)로 결과를 보여주므로, 다시 정렬합니다.
        std::sort(final_results.begin(), final_results.end(), [](const auto& a, const auto& b) {
            return a.score > b.score;
        });

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;

        std::stringstream ss;
        ss << "{";
        ss << "\"time_ms\": " << std::fixed << std::setprecision(4) << elapsed_ms.count() << ",";
        ss << "\"results\": [";

        for (int i = 0; i < final_results.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "{"
               << "\"index\":" << final_results[i].index << ","
               << "\"score\":" << std::fixed << std::setprecision(4) << final_results[i].score
               << "}";
        }
        ss << "] }";

        std::string json_str = ss.str();
        char* out = new char[json_str.size() + 1];
        memcpy(out, json_str.c_str(), json_str.size() + 1);
        return out;
    }

    /**
     * @brief `search_emojis` 함수에서 할당된 메모리를 해제합니다.
     *        JavaScript 측에서 이 함수를 호출하여 C++에서 할당한 메모리 누수를 방지해야 합니다.
     * @param ptr `search_emojis` 함수가 반환한 문자열 포인터
     */
    void free_result_memory(const char* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
}
