// JavaScript로 이모지 유사도를 계산하는 로직

/**
 * 벡터의 크기(magnitude)를 계산합니다.
 * @param {number[]} vec 
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
 * @param {number[]} vecA 
 * @param {number[]} vecB 
 * @returns {number}
 */
function cosineSimilarityJS(vecA, vecB) {
    let dotProduct = 0;
    for (let i = 0; i < vecA.length; i++) {
        dotProduct += vecA[i] * vecB[i];
    }
    const magA = calculateMagnitudeJS(vecA);
    const magB = calculateMagnitudeJS(vecB);
    
    if (magA === 0 || magB === 0) {
        return 0;
    }
    return dotProduct / (magA * magB);
}

/**
 * 쿼리 벡터와 전체 임베딩 데이터를 사용하여 유사도 검색을 수행합니다.
 * @param {number[]} queryVector 
 * @returns {Array<{index: number, score: number}>}
 */
function searchEmojisJS(queryVector) {
    if (!window.EMBEDDINGS || window.EMBEDDINGS.length === 0) {
        console.error("EMBEDDINGS 데이터가 로드되지 않았습니다.");
        return [];
    }

    const results = [];
    for (let i = 0; i < window.EMBEDDINGS.length; i++) {
        const score = cosineSimilarityJS(window.EMBEDDINGS[i], queryVector);
        results.push({ index: i, score: score });
    }

    // 점수가 높은 순으로 정렬
    results.sort((a, b) => b.score - a.score);

    // 상위 10개 결과 반환
    return results.slice(0, 5);
}
