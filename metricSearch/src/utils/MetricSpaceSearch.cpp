#include "../../include/utils/MetricSpaceSearch.h"
#include <map>
#include <string>

//用于存储距离的哈希表
unordered_map<MetricData*, unordered_map<MetricData*, long double>> MetricSpaceSearch::pivotDistanceCache;
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

MetricSpaceSearch::SearchResult MetricSpaceSearch::nearestNeighbor(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    const std::shared_ptr<MetricData>& query,
    const std::shared_ptr<MetricDistance>& distanceFunc,
    const std::shared_ptr<MetricData>& pivot)
{
    const long double EPS = 1e-15;

    SearchResult result;
    result.pivot = pivot;
    result.distanceFunc = distanceFunc;
    result.calculations = 0;

    if (dataset.empty()) {
        return result; // 数据集为空，直接返回空结果
    }

    // 如果未指定支撑点，则默认使用第一个数据点作为支撑点
    std::shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // d(p, q)：支撑点到查询点的距离（优先从缓存中获取）
    long double d_pq;
    if (pivotDistanceCache.count(initialPivot.get()) &&
        pivotDistanceCache[initialPivot.get()].count(query.get()))
    {
        d_pq = pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else
    {
        cout << "\n----------------[DEBUG]:预处理不完全----------------\n";
        d_pq = distanceFunc->distance(*initialPivot, *query);
        pivotDistanceCache[initialPivot.get()][query.get()] = d_pq;
    }

    // 初始化最近邻为无穷大和空指针
    result.distance = std::numeric_limits<long double>::max();
    result.nearest.reset();

    // === 直接选择第一个非支撑点、非查询点作为初始最近邻 ===
    bool foundInitial = false;
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query) {
            continue; // 跳过支撑点和查询点
        }

        // 找到第一个符合条件的点
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;
        result.distance = d_qx;
        result.nearest = data;
        result.steps.push_back("初始化最近邻为: " + data->toString() +
            "，初始距离为: " + std::to_string(d_qx));
        foundInitial = true;
        break;
    }

    if (!foundInitial) {
        result.steps.push_back("警告：没有找到可用的初始最近邻！");
        return result;
    }

    // === 开始对整个数据集进行搜索 ===
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query || data == result.nearest) {
            continue; // 跳过支撑点、查询点和当前最近邻
        }

        // 获取 d(p, x)，即支撑点到当前数据点的距离
        long double d_px;
        if (pivotDistanceCache.count(initialPivot.get()) &&
            pivotDistanceCache[initialPivot.get()].count(data.get()))
        {
            d_px = pivotDistanceCache[initialPivot.get()][data.get()];
        }
        else
        {
            d_px = distanceFunc->distance(*initialPivot, *data);
            pivotDistanceCache[initialPivot.get()][data.get()] = d_px;
        }

        // ====== 使用三角不等式进行剪枝 ======
        long double lower_bound = std::abs(d_pq - d_px);   // 下界
        long double upper_bound = d_pq + d_px;             // 上界

        // Debug 输出剪枝信息
        result.steps.push_back("[DEBUG] 当前点: " + data->toString() +
            ", d_px=" + std::to_string(d_px) +
            ", lower_bound=" + std::to_string(lower_bound) +
            ", upper_bound=" + std::to_string(upper_bound) +
            ", current_min=" + std::to_string(result.distance));

        // 上界剪枝：如果上界小于当前最小距离，跳过
        if (upper_bound < result.distance - EPS) {
            result.steps.push_back("由于上界剪枝跳过: " + data->toString());
            continue;
        }

        // 下界剪枝：如果下界大于等于当前最小距离，跳过
        if (lower_bound >= result.distance + EPS) {
            result.steps.push_back("由于下界剪枝跳过: " + data->toString());
            continue;
        }

        // 无法剪枝，必须计算真实距离
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;

        result.steps.push_back("计算查询点到 " + data->toString() +
            " 的实际距离: " + std::to_string(d_qx));

        // 更新最近邻
        if (d_qx < result.distance) {
            result.distance = d_qx;
            result.nearest = data;
            result.steps.push_back("发现新的最近邻: " + data->toString() +
                " 最近距离为 " + std::to_string(d_qx));
        }
    }

    //最后判断支撑点是不是最近邻，因为之前跳过了支撑点
    if (d_pq < result.distance) {
        result.distance = d_pq;
        result.nearest = initialPivot;
        result.steps.push_back("发现新的最近邻: 是支撑点！最近距离为 " + std::to_string(d_pq));
    }


    return result;
}

//性能分析
void MetricSpaceSearch::analyzePerformance(
    const vector<shared_ptr<MetricData>>& dataset,
    const vector<vector<SearchResult>>& allResults,
    const shared_ptr<MetricData>& query) {

    cout << "调试：进入分析函数，数据集大小=" << dataset.size()
        << "，结果组数=" << allResults.size() << endl;

    if (dataset.empty() || allResults.empty()) return;

    cout << "\n===== 综合性能分析 =====" << endl;
    cout << "查询点: " << query->toString() << endl;

    struct PivotStat {
        shared_ptr<MetricData> pivot;
        map<string, long double> avgFilterRates;
        map<string, long double> avgTimes;
        map<string, int> totalCalcs;
    };
    vector<PivotStat> pivotStats;

    const int maxPossibleCalcs = dataset.size();

    // 收集统计数据
    for (const auto& roundResults : allResults) {
        if (roundResults.empty()) continue;

        auto pivot = roundResults[0].pivot;
        bool found = false;

        PivotStat* stat = nullptr;
        for (auto& ps : pivotStats) {
            if (ps.pivot == pivot) {
                stat = &ps;
                found = true;
                break;
            }
        }

        if (!found) {
            pivotStats.push_back({ pivot });
            stat = &pivotStats.back();
        }

        for (const auto& result : roundResults) {
            string funcName = result.distanceFunc->getName();

            if (stat->avgFilterRates.find(funcName) == stat->avgFilterRates.end()) {
                stat->avgFilterRates[funcName] = 0.0;
                stat->avgTimes[funcName] = 0.0;
                stat->totalCalcs[funcName] = 0;
            }

            stat->avgFilterRates[funcName] += 1.0 - (long double)(result.calculations) / maxPossibleCalcs;
            stat->avgTimes[funcName] += result.timeMicrosec;
            stat->totalCalcs[funcName] += result.calculations;
        }
    }

    // 平均化
    for (auto& stat : pivotStats) {
        int rounds = allResults.size();

        // 替换结构化绑定：for (auto& [name, count] : stat.totalCalcs)
        for (const auto& pair : stat.totalCalcs) {
            const string& name = pair.first;
            stat.avgFilterRates[name] /= rounds;
            stat.avgTimes[name] /= rounds;
        }
    }

    // 输出比较结果
    cout << "\n1. 支撑点效率对比:" << endl;
    for (const auto& stat : pivotStats) {
        cout << "支撑点 " << stat.pivot->toString() << ":" << endl;

        // 替换结构化绑定：for (const auto& [name, rate] : stat.avgFilterRates)
        for (const auto& pair : stat.avgFilterRates) {
            const string& name = pair.first;
            long double rate = pair.second;

            cout << "  [" << name << "]"
                << " | 平均过滤效率: " << rate * 100 << "%"
                << " | 平均耗时: " << stat.avgTimes.at(name) << " μs"
                << " | 总计算次数: " << stat.totalCalcs.at(name) << "/" << maxPossibleCalcs
                << endl;
        }
    }

    // 找出最优支撑点和最优距离函数
    struct {
        shared_ptr<MetricData> pivot;
        string bestFunc;
        int minTotal = numeric_limits<int>::max();
    } bestOverall;

    for (const auto& stat : pivotStats) {
        // 替换结构化绑定：for (const auto& [name, total] : stat.totalCalcs)
        for (const auto& pair : stat.totalCalcs) {
            const string& name = pair.first;
            int total = pair.second;

            if (total < bestOverall.minTotal) {
                bestOverall.pivot = stat.pivot;
                bestOverall.bestFunc = name;
                bestOverall.minTotal = total;
            }
        }
    }

    cout << "\n2. 最优支撑点: " << bestOverall.pivot->toString()
        << " 使用 \"" << bestOverall.bestFunc << "\""
        << " (最少计算次数: " << bestOverall.minTotal << ")" << endl;

    // 距离函数对比
    map<string, int> totalCalculations;
    for (const auto& roundResults : allResults) {
        for (const auto& result : roundResults) {
            string name = result.distanceFunc->getName();
            totalCalculations[name] += result.calculations;
        }
    }

    cout << "\n3. 距离函数对比:" << endl;
    // 替换结构化绑定：for (const auto& [name, total] : totalCalculations)
    for (const auto& pair : totalCalculations) {
        const string& name = pair.first;
        int total = pair.second;

        cout << "  [" << name << "] 总计算次数: " << total << endl;
    }

    auto bestFunc = min_element(totalCalculations.begin(), totalCalculations.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second < b.second;
        });

    cout << "推荐使用: " << bestFunc->first << endl;
}




