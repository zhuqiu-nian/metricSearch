#include "../../include/utils/MetricSpaceSearch.h"

//用于存储距离的哈希表
unordered_map<MetricData*, unordered_map<MetricData*, double>> MetricSpaceSearch::pivotDistanceCache;
bool MetricSpaceSearch::isPrecomputed = false;
size_t MetricSpaceSearch::cachedDatasetSize = 0;

//预计算所有支撑点到数据点的距离
//把预处理函数改写成模板函数了，此函数移入了头文件定义里

// 清理缓存
void MetricSpaceSearch::clearCache() {
    pivotDistanceCache.clear();
    isPrecomputed = false;
    cachedDatasetSize = 0;
    std::cout << "[系统] 已清空距离缓存" << std::endl;
}

// //基于三角不等式的度量空间最近邻搜索算法
MetricSpaceSearch::SearchResult MetricSpaceSearch::nearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    const shared_ptr<MetricData>& pivot) {

    SearchResult result;
    result.pivot = pivot;
    result.calculations = 0;

    if (dataset.empty()) {
        return result;
    }

    // 如果没有指定支撑点，使用第一个元素
    shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // 从缓存中读取 d_pq（若不存在则计算）
    double d_pq;
    if (pivotDistanceCache.count(initialPivot.get())) {
        d_pq = pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else {
        d_pq = distanceFunc->distance(*initialPivot, *query);
    }

    // 假设支撑点为初始最近邻
    result.nearest = initialPivot;
    result.distance = d_pq;

    // 遍历数据集
    for (const auto& data : dataset) {
        if (data == initialPivot) {
            continue; // 已经计算过
        }

        // 应用三角不等式进行过滤
        double d_px = pivotDistanceCache[initialPivot.get()][data.get()];

        //注意，p是查询点，q是支撑点，x是数据点，计算逻辑如下：
        //先初始化查询点与支撑点距离pq，对于每一个数据，先计算查支撑点到数据点的距离px，
        if (d_px <= d_pq + result.distance && d_px >= abs(d_pq - result.distance)) {
            // 可能更近，需要计算实际距离
            double d_qx = distanceFunc->distance(*query, *data);
            result.calculations++;
            result.steps.push_back("计算查询点到 " +
                data->toString() + " 的实际距离: " + to_string(d_qx));

            if (d_qx < result.distance) {
                result.nearest = data;
                result.distance = d_qx;
                result.steps.push_back("发现新的最近邻: " +
                    data->toString() + " 最近距离为 " +
                    to_string(d_qx));
            }


        }
        else {
            result.steps.push_back("由于三角不等式，跳过对 " + data->toString() +
                " 的完整计算");
        }
    }

    return result;
}

//性能分析
void MetricSpaceSearch::analyzePerformance(
    const vector<shared_ptr<MetricData>>& dataset,
    const vector<vector<SearchResult>>& allResults,
    const shared_ptr<MetricData>& query) {

    //调试
    cout << "调试：进入分析函数，数据集大小=" << dataset.size()
        << "，结果组数=" << allResults.size() << endl;



    if (dataset.empty() || allResults.empty()) return;

    cout << "\n===== 综合性能分析 =====" << endl;
    cout << "查询点: " << query->toString() << endl;

    // 统计所有支撑点的表现
    struct PivotStat {
        shared_ptr<MetricData> pivot;
        double avgFilterRate;
        double avgTime;
        int totalCalcs;
    };
    vector<PivotStat> pivotStats;

    // 计算理论最大计算次数
    const int maxPossibleCalcs = (dataset.size() - 1) * 2 + 1; // (n-1个点)*2种距离+最开始时支撑点到查询点的距离

    // 收集统计数据
    // 修改收集统计数据的部分
    for (const auto& roundResults : allResults) {
        if (roundResults.size() != 2) continue;

        const auto& euclideanResult = roundResults[0];
        const auto& lonePointResult = roundResults[1];

        // 直接从结果中获取pivot（需要在nearestNeighbor函数中保存pivot信息）
        auto pivot = euclideanResult.pivot;

        double realFilterRate = 1.0 - (double)(euclideanResult.calculations + lonePointResult.calculations) /
            (maxPossibleCalcs * 2);

        pivotStats.push_back({
            pivot,
            realFilterRate,
            (euclideanResult.timeMicrosec + lonePointResult.timeMicrosec) / 2.0,
            euclideanResult.calculations + lonePointResult.calculations
            });
    }

    // 输出比较结果
    cout << "\n1. 支撑点效率对比:" << endl;
    for (const auto& stat : pivotStats) {
        cout << "支撑点 " << stat.pivot->toString()
            << " | 平均过滤效率: " << stat.avgFilterRate * 100 << "%"
            << " | 平均耗时: " << stat.avgTime << " μs"
            << " | 总计算次数: " << stat.totalCalcs << "/" << maxPossibleCalcs * 2
            << endl;
    }

    // 找出最优支撑点
    auto bestPivot = min_element(pivotStats.begin(), pivotStats.end(),
        [](const PivotStat& a, const PivotStat& b) {
            return a.totalCalcs < b.totalCalcs;
        });

    cout << "\n2. 最优支撑点: " << bestPivot->pivot->toString()
        << " (最少计算次数: " << bestPivot->totalCalcs << ")" << endl;

    // 距离函数比较
    int euclideanTotal = 0, lonePointTotal = 0;
    for (const auto& roundResults : allResults) {
        euclideanTotal += roundResults[0].calculations;
        lonePointTotal += roundResults[1].calculations;
    }
    cout << "\n3. 距离函数对比:" << endl;
    cout << "欧几里得距离总计算次数: " << euclideanTotal << endl;
    cout << "孤点距离总计算次数: " << lonePointTotal << endl;
    cout << "推荐使用: "
        << (euclideanTotal < lonePointTotal ? "欧几里得距离" : "孤点距离")
        << endl;
}

