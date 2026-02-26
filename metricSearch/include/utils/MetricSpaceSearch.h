#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <chrono>
#include <memory>
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"

class MetricSpaceSearch {
public:
    struct SearchResult {
        // 所有查询共用字段
        shared_ptr<MetricData> nearest;// 找到的最近邻数据点
        shared_ptr<MetricData> pivot;// 使用的支撑点
        long double distance;// 查询点到最近邻的距离
        int calculations; // 距离计算的次数
        long timeMicrosec;// 搜索耗时(微秒)
        vector<string> steps;// 搜索过程的步骤记录
        shared_ptr<MetricDistance> distanceFunc; // 使用的距离函数

        // 范围查询相关字段
        vector<pair<shared_ptr<MetricData>, long double>> rangeResults; // <数据点, 距离>

        // kNN 查询相关字段
        vector<pair<shared_ptr<MetricData>, long double>> knnResults;   // <数据点, 距离>
        int k = 0;                              // 请求的 k 值

        // 距离受限的 kNN 查询相关字段
        vector<pair<shared_ptr<MetricData>, long double>> boundedKnnResults; // <数据点, 距离>
        int boundedK = 0;                       // 请求的 boundedK 值
        long double radius = numeric_limits<long double>::max(); // 最大搜索半径

        // 构造函数
        SearchResult() :
            distance(numeric_limits<long double>::max()),
            calculations(0),
            timeMicrosec(0),
            k(0),
            boundedK(0),
            radius(numeric_limits<long double>::max()) {}
    };

    //用于存储距离的哈希表
    static unordered_map<MetricData*, unordered_map<MetricData*, long double>> pivotDistanceCache;//存储单个缓存
    static bool isPrecomputed;
    static size_t cachedDatasetSize;


    // 模板化预处理函数，为单个距离函数计算缓存
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

    //分析搜索性能
    static void analyzePerformance(
        const vector<shared_ptr<MetricData>>& dataset,
        const vector<vector<SearchResult>>& allResults,
        const shared_ptr<MetricData>& query);

    static void analyzePerformanceForAll(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::vector<std::vector<std::vector<SearchResult>>>& allResults,
        const std::vector<std::shared_ptr<MetricData>>& queries);


    //硬编码每个组的距离函数
    static vector<shared_ptr<MetricDistance>> getAllDistanceFunctions(int data_var) {
        if (data_var == 1) {
            // 向量数据支持的距离函数
            return {
                make_shared<ManhattanDistance>(),
                make_shared<EuclideanDistance>(),
                make_shared<ChebyshevDistance>(),
                make_shared<LonePointDistance>()
            };
        }
        else if (data_var == 2) {
            // 字符串数据支持的距离函数
            return {
                make_shared<EditDistance>()
            };
        }
        else if (data_var == 3) {
            // 蛋白质数据支持的距离函数
            return {
                make_shared<WeightedEditDistance>()
            };
        }
        return {};
    }

    //硬编码每个组的距离函数
    static shared_ptr<MetricDistance> createDistanceFunction(int distanceType, int data_var) {
        if (data_var == 1) {
            // 向量数据
            switch (distanceType) {
            case 1: return make_shared<EuclideanDistance>();
            case 2: return make_shared<ChebyshevDistance>();
            case 3: return make_shared<LonePointDistance>();
            case 4: return make_shared<ManhattanDistance>();
            default:
                throw invalid_argument("不支持的向量距离函数类型: " + to_string(distanceType));
            }
        }
        else if (data_var == 2) {
            // 字符串数据
            switch (distanceType) {
            case 1: return make_shared<EditDistance>();
            case 2: return make_shared<HammingDistance>();
            default:
                throw invalid_argument("不支持的字符串距离函数类型: " + to_string(distanceType));
            }
        }
        else if (data_var == 3) {
            // 蛋白质序列数据
            switch (distanceType) {
            case 1: return make_shared<WeightedEditDistance>();
            default:
                throw invalid_argument("不支持的蛋白质序列距离函数类型: " + to_string(distanceType));
            }
        }
        else {
            throw invalid_argument("未知的数据类型: " + to_string(data_var));
        }
    }
};
