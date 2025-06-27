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