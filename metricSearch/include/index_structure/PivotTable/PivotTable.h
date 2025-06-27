#pragma once

#include "../../interfaces/MetricData.h"
#include "../../interfaces/MetricDistance.h"

#include <vector>
#include <memory>
#include <string>

class PivotTable {
public:
    // 批建构造函数（新增 distanceType 和 data_var 参数）
    PivotTable(const std::vector<std::shared_ptr<MetricData>>& allData,
        const std::vector<int>& pivotIndices,
        int distanceType,
        int data_var);

    PivotTable(const vector<shared_ptr<MetricData>>& allData,
        int k,
        int distanceType,
        int data_var);

    // 基于三角不等式的搜索接口
    std::vector<std::shared_ptr<MetricData>> search(
        const MetricData& query,
        long double threshold,
        long long* distanceCount = nullptr) const;  // 可选参数，默认为 nullptr

    // 辅助函数：随机选取 k 个支撑点索引
    vector<int> selectRandomPivots(int totalSize, int k) const;

    //外部接口，调用PivotTable进行相关查询
    // 新增静态方法：交互式范围查询
    static void interactiveRangeSearch(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // 获取和重置内部距离计算次数
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    std::vector<std::shared_ptr<MetricData>> data_;          // 非支撑点数据
    std::vector<std::shared_ptr<MetricData>> pivots_;         // 支撑点集合
    std::vector<std::vector<long double>> distanceTable_;     // 距离表：pivots × data_
    std::shared_ptr<MetricDistance> distance_;                // 动态选择的距离函数
    vector<int> selectedPivotIndices_;  // 记录最终选中的支撑点索引（可输出）

    // 辅助函数：构建距离表并计时
    void buildDistanceTable(int distanceType, int data_var);

    // 新增：构建空或极小数据集的 PivotTable
    void buildEmpty(int distanceType, int data_var);

    static long long distanceCalculations_;  // 新增：类内距离计算次数统计

};