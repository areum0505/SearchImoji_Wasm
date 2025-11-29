#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <chrono>

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

        // 유사도 계산 시간 측정 시작
        //auto t_calc_start = std::chrono::high_resolution_clock::now();

        std::vector<SearchResult> results;
        results.reserve(NUM_EMBEDDINGS);

        for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
            double score = cosineSimilarity(EMBEDDINGS[i], query_vector);
            results.push_back({ i, score });
        }

        // 유사도 계산 시간 측정 종료
        //auto t_calc_end = std::chrono::high_resolution_clock::now();

        // 정렬 시간 측정 시작
        //auto t_sort_start = std::chrono::high_resolution_clock::now();

        std::sort(results.begin(), results.end(),
            [](const SearchResult& a, const SearchResult& b) {
                return a.score > b.score;
            });
        
        // 정렬 시간 측정 종료
        //auto t_sort_end = std::chrono::high_resolution_clock::now();

        // 시간 계산 (밀리초 단위)
        //std::chrono::duration<double, std::milli> calc_ms = t_calc_end - t_calc_start;
        //std::chrono::duration<double, std::milli> sort_ms = t_sort_end - t_sort_start;
        //double total_ms = calc_ms.count() + sort_ms.count();

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;

        std::stringstream ss;
        ss << "{";
        //ss << "\"calc_ms\": " << std::fixed << std::setprecision(4) << calc_ms.count() << ",";
        //ss << "\"sort_ms\": " << std::fixed << std::setprecision(4) << sort_ms.count() << ",";
        ss << "\"time_ms\": " << std::fixed << std::setprecision(4) << elapsed_ms.count() << ",";
        ss << "\"results\": [";

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

    void free_result_memory(const char* ptr) {
        if (ptr) {
            delete[] ptr;
        }
    }
}
