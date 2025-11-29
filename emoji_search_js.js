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
 * @returns {object} { results: Array<{index: number, score: number}>, time_ms: number }
 */
function searchEmojisJS(queryVector) {
    const start_time = performance.now();
    if (!window.EMBEDDINGS || window.EMBEDDINGS.length === 0) {
        console.error("EMBEDDINGS 데이터가 로드되지 않았습니다.");
        return { results: [], time_ms: 0 };
    }

    // 1. 유사도 계산 시간 측정 시작
    //const t_calc_start = performance.now();

    const results = [];
    for (let i = 0; i < window.EMBEDDINGS.length; i++) {
        const score = cosineSimilarityJS(window.EMBEDDINGS[i], queryVector);
        results.push({ index: i, score: score });
    }

    //const t_calc_end = performance.now(); // 계산 끝

    // 2. 정렬 시간 측정 시작
    //const t_sort_start = performance.now();

    // 점수가 높은 순으로 정렬
    results.sort((a, b) => b.score - a.score);

    //const t_sort_end = performance.now(); // 정렬 끝

    // 시간 계산
    //const calc_ms = t_calc_end - t_calc_start;
    //const sort_ms = t_sort_end - t_sort_start;
    //const time_ms = calc_ms + sort_ms;

    // 상위 10개 결과 반환
    const topResults = results.slice(0, 5);

    const end_time = performance.now();
    const time_ms = end_time - start_time;

    return { 
        results: topResults, 
        //calc_ms: calc_ms, 
        //sort_ms: sort_ms, 
        time_ms: time_ms 
    };
}
