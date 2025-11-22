#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <fstream>  // 파일 입출력 필수
#include <sstream>

// emoji.hpp: 1200+개의 이모지 벡터 데이터
#include "emoji.hpp"

const int EMBEDDING_DIM = 768;
const int NUM_EMBEDDINGS = sizeof(EMBEDDINGS) / sizeof(EMBEDDINGS[0]);

struct EmojiMetadata {
    std::string character;
    std::string description;
};

struct SearchResult {
    int index;
    double score;
};

// --- [1] CSV 로드 함수 (기존과 동일) ---
std::vector<EmojiMetadata> loadEmojiCSV(const std::string& filename) {
    std::vector<EmojiMetadata> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: 'emoji.csv' 파일을 찾을 수 없습니다." << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string segment;
        EmojiMetadata meta;

        if (std::getline(ss, segment, ',')) {
            meta.character = segment;
        }
        if (std::getline(ss, segment)) {
            size_t first = segment.find_first_not_of(" \"");
            size_t last = segment.find_last_not_of(" \"");
            if (first != std::string::npos && last != std::string::npos) {
                meta.description = segment.substr(first, last - first + 1);
            } else {
                meta.description = segment;
            }
        }
        data.push_back(meta);
    }
    return data;
}

// --- [2] test.txt에서 쿼리 벡터 읽기 (새로 추가된 부분) ---
std::vector<double> loadQueryVector(const std::string& filename) {
    std::vector<double> vec;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: '" << filename << "' 파일을 열 수 없습니다." << std::endl;
        return vec; // 빈 벡터 반환
    }

    double val;
    // 파일에서 숫자(실수)를 하나씩 읽어서 벡터에 추가
    // 공백, 줄바꿈, 탭 등은 자동으로 무시하고 숫자만 읽습니다.
    while (file >> val) {
        vec.push_back(val);
    }

    file.close();
    return vec;
}

// --- [3] 수학 연산 함수들 ---
double calculateMagnitude(const double* vec) {
    double sum = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i) sum += vec[i] * vec[i];
    return std::sqrt(sum);
}

double calculateMagnitude(const std::vector<double>& vec) {
    double sum = 0.0;
    for (double val : vec) sum += val * val;
    return std::sqrt(sum);
}

double cosineSimilarity(const double* dbVec, const std::vector<double>& queryVec) {
    double dotProduct = 0.0;
    for (int i = 0; i < EMBEDDING_DIM; ++i) {
        dotProduct += dbVec[i] * queryVec[i];
    }
    double magA = calculateMagnitude(dbVec);
    double magB = calculateMagnitude(queryVec);
    if (magA == 0.0 || magB == 0.0) return 0.0;
    return dotProduct / (magA * magB);
}

// --- Main ---
int main() {
    // 1. CSV 데이터 로드
    std::vector<EmojiMetadata> emojiData = loadEmojiCSV("emoji.csv");
    if (emojiData.size() != NUM_EMBEDDINGS) {
        std::cerr << "Error: 데이터 개수 불일치 (CSV vs HPP)" << std::endl;
        return 1;
    }

    // 2. test.txt에서 쿼리 벡터 로드
    std::cout << "Reading query vector from 'test.txt'..." << std::endl;
    std::vector<double> queryVec = loadQueryVector("test.txt");

    // 3. 벡터 유효성 검사
    if (queryVec.size() != EMBEDDING_DIM) {
        std::cerr << "Error: 입력 벡터 차원 오류!" << std::endl;
        std::cerr << " - Expected: " << EMBEDDING_DIM << std::endl;
        std::cerr << " - Actual: " << queryVec.size() << std::endl;
        return 1;
    }

    // 4. 유사도 계산
    std::vector<SearchResult> results;
    results.reserve(NUM_EMBEDDINGS);

    for (int i = 0; i < NUM_EMBEDDINGS; ++i) {
        double score = cosineSimilarity(EMBEDDINGS[i], queryVec);
        results.push_back({i, score});
    }

    // 5. 정렬 (높은 점수 순)
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.score > b.score;
    });

    // 6. 결과 출력
    std::cout << "\n--- Best Matching Emojis ---\n";
    for (int i = 0; i < 5 && i < NUM_EMBEDDINGS; ++i) {
        int idx = results[i].index;
        std::cout << (i + 1) << ". " << emojiData[idx].character 
                  << "  " << emojiData[idx].description 
                  << " \t[Score: " << std::fixed << std::setprecision(4) << results[i].score << "]" 
                  << std::endl;
    }

    return 0;
}