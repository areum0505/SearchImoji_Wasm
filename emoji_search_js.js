// JavaScript로 이모지 유사도를 계산하는 로직

// --- 성능 최적화를 위한 전역 캐시 변수 ---
// EMBEDDINGS 데이터의 벡터 크기를 미리 계산하여 저장해두는 캐시입니다.
// 이 값들은 한 번 계산된 후에는 계속 재사용되어 중복 계산을 방지합니다.
let EMBEDDING_MAGNITUDES_JS = [];

/**
 * 벡터의 크기(magnitude)를 계산합니다.
 * @param {number[]} vec - 크기를 계산할 벡터
 * @returns {number}
 */
function calculateMagnitudeJS(vec) {
    let sum = 0;
    for (let i = 0; i < vec.length; i++) {
        sum += vec[i] * vec[i];
    }
    return Math.sqrt(sum);
}

/**
 * 두 벡터 간의 코사인 유사도를 계산합니다.
 * @param {number[]} vecA - 첫 번째 벡터
 * @param {number} magA - 첫 번째 벡터의 크기
 * @param {number[]} vecB - 두 번째 벡터
 * @param {number} magB - 두 번째 벡터의 크기
 * @returns {number}
 */
function cosineSimilarityJS(vecA, magA, vecB, magB) {
    let dotProduct = 0;
    for (let i = 0; i < vecA.length; i++) {
        dotProduct += vecA[i] * vecB[i];
    }
    
    if (magA === 0 || magB === 0) {
        return 0;
    }
    return dotProduct / (magA * magB);
}

/**
 * 쿼리 벡터와 전체 임베딩 데이터를 사용하여 유사도 검색을 수행합니다.
 * @param {number[]} queryVector - 사용자가 입력한 텍스트의 임베딩 벡터
 * @returns {object} { results: Array<{index: number, score: number}>, time_ms: number }
 */
function searchEmojisJS(queryVector) {
    const start_time = performance.now();
    if (!window.EMBEDDINGS || window.EMBEDDINGS.length === 0) {
        console.error("EMBEDDINGS 데이터가 로드되지 않았습니다.");
        return { results: [], time_ms: 0 };
    }

    // --- 1. 크기(magnitude) 계산 최적화 ---

    // 데이터베이스 임베딩 크기 캐시 초기화 (최초 1회만 실행)
    if (EMBEDDING_MAGNITUDES_JS.length === 0) {
        for (let i = 0; i < window.EMBEDDINGS.length; i++) {
            EMBEDDING_MAGNITUDES_JS.push(calculateMagnitudeJS(window.EMBEDDINGS[i]));
        }
    }

    // 쿼리 벡터의 크기를 루프 밖에서 한 번만 계산
    const queryMag = calculateMagnitudeJS(queryVector);


    // --- 2. Top-N 검색 로직 ---
    const TOP_N_JS = 5;
    const topResults = [];

    for (let i = 0; i < window.EMBEDDINGS.length; i++) {
        // 미리 계산된 크기 값을 사용하여 유사도 계산
        const score = cosineSimilarityJS(window.EMBEDDINGS[i], EMBEDDING_MAGNITUDES_JS[i], queryVector, queryMag);
        
        if (topResults.length < TOP_N_JS) {
            topResults.push({ index: i, score: score });
        } else {
             // 현재 점수가 가장 낮은 상위 결과(배열의 마지막 요소)보다 높으면 교체
            if (score > topResults[TOP_N_JS - 1].score) {
                topResults.pop(); // 가장 낮은 점수 제거
                topResults.push({ index: i, score: score });
                // N이 작으므로, 삽입 후 매번 정렬하는 것이 splice보다 간단하고 충분히 빠릅니다.
                topResults.sort((a, b) => b.score - a.score);
            }
        }
    }

    // 최종적으로 한 번 더 정렬
    topResults.sort((a, b) => b.score - a.score);
    
    const end_time = performance.now();
    const time_ms = end_time - start_time;

    return { 
        results: topResults, 
        time_ms: time_ms 
    };
}
