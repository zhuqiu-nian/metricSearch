#pragma once
#include <vector>
#include <unordered_map>
#include <chrono>
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"

using namespace std;

class MetricSpaceSearch {
public:
    struct SearchResult {
        shared_ptr<MetricData> nearest;// 找到的最近邻数据点
        shared_ptr<MetricData> pivot;// 使用的支撑点
        double distance;// 查询点到最近邻的距离
        int calculations; // 距离计算的次数
        long timeMicrosec;// 搜索耗时(微秒)
        vector<string> steps;// 搜索过程的步骤记录

        // 添加构造函数
        SearchResult() :
            distance(numeric_limits<double>::max()),
            calculations(0),
            timeMicrosec(0) {}
    };

    //用于存储距离的哈希表
    static unordered_map<MetricData*, unordered_map<MetricData*, double>> pivotDistanceCache;
    static bool isPrecomputed;
    static size_t cachedDatasetSize;


    // 模板化预处理函数
    template<typename T>
    static void precomputeDistances(
        const std::vector<std::shared_ptr<T>>& dataset,
        const std::shared_ptr<MetricDistance>& distanceFunc) {

        // 编译时类型检查
        static_assert(std::is_base_of_v<MetricData, T>,
            "Template type T must inherit from MetricData");

        //开始预处理
        clearCache();
        std::cout << "[预处理] 开始计算支撑点距离..." << std::endl;
        std::cout << "[预处理] 数据集大小: " << dataset.size() << std::endl;
        auto start = chrono::high_resolution_clock::now();

        pivotDistanceCache.clear();
        for (const auto& pivot : dataset) {
            for (const auto& data : dataset) {
                if (pivot != data) {
                    pivotDistanceCache[pivot.get()][data.get()] =
                        distanceFunc->distance(*pivot, *data);
                }
            }
        }

        //预处理完成
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start);

        isPrecomputed = true;
        cachedDatasetSize = dataset.size();

        std::cout << "[预处理] 完成！" << std::endl;
        std::cout << "[预处理] 缓存条目数: " << pivotDistanceCache.size() << std::endl;
        std::cout << "[预处理] 耗时: " << duration.count() << " ms" << std::endl;
    }

    // 清理缓存
    static void clearCache();

    //基于三角不等式的度量空间最近邻搜索算法
    static SearchResult nearestNeighbor(
        const vector<shared_ptr<MetricData>>& dataset,
        const shared_ptr<MetricData>& query,
        const shared_ptr<MetricDistance>& distanceFunc,
        const shared_ptr<MetricData>& pivot = nullptr);

    //分析搜索性能
    static void analyzePerformance(
        const vector<shared_ptr<MetricData>>& dataset,
        const vector<vector<SearchResult>>& allResults,
        const shared_ptr<MetricData>& query);


};
