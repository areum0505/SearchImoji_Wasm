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

    const TOP_N_JS = 5;
    const topResults = [];

    // 1. 유사도 계산 시간 측정 시작
    //const t_calc_start = performance.now();
    for (let i = 0; i < window.EMBEDDINGS.length; i++) {
        const score = cosineSimilarityJS(window.EMBEDDINGS[i], queryVector);
        const currentResult = { index: i, score: score };

        if (topResults.length < TOP_N_JS) {
            topResults.push(currentResult);
            // 배열이 TOP_N_JS개로 채워지면 초기 정렬
            if (topResults.length === TOP_N_JS) {
                topResults.sort((a, b) => b.score - a.score);
            }
        } else {
            // 현재 점수가 가장 낮은 상위 결과보다 높으면 교체
            if (score > topResults[TOP_N_JS - 1].score) {
                topResults.pop(); // 가장 낮은 점수 제거

                // 새 결과를 삽입할 위치를 찾아 삽입 (정렬 유지)
                let inserted = false;
                for (let j = 0; j < TOP_N_JS - 1; j++) {
                    if (currentResult.score >= topResults[j].score) {
                        topResults.splice(j, 0, currentResult);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) { // 만약 끝까지 삽입되지 않았다면 (가장 작은 값 다음)
                    topResults.push(currentResult);
                }
            }
        }
    }

    //const t_calc_end = performance.now(); // 계산 끝

    // 2. 정렬 시간 측정 시작
    //const t_sort_start = performance.now();

    // 이모지 개수가 TOP_N_JS보다 적을 경우를 위한 최종 정렬
    if (topResults.length < TOP_N_JS) {
        topResults.sort((a, b) => b.score - a.score);
    }

    //const t_sort_end = performance.now(); // 정렬 끝

    // 시간 계산
    //const calc_ms = t_calc_end - t_calc_start;
    //const sort_ms = t_sort_end - t_sort_start;
    //const time_ms = calc_ms + sort_ms;
    const end_time = performance.now();
    const time_ms = end_time - start_time;

    return { 
        results: topResults, 
        //calc_ms: calc_ms, 
        //sort_ms: sort_ms, 
        time_ms: time_ms 
    };
}
