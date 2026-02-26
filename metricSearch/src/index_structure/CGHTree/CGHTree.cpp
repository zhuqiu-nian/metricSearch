// CGHTree.cpp
#include "../../../include/index_structure/CGHTree/CGHTree.h"
#include "../../../include/index_structure/CGHTree/CGHTInternalNode.h"
#include "../../../include/index_structure/CGHTree/CGHTLeafNode.h"
#include "../../../include/index_structure/CGHTree/CGHTree.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>

const int MaxCGHTLeafSize = 20;

long long CGHTree::distanceCalculations_ = 0;

std::unique_ptr<CGHTNode> CGHTree::bulkLoad(
    const std::vector<DataPtr>& data,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method,
    int f /* 新增：划分次数 */)
{
    if (data.size() <= MaxCGHTLeafSize) {
        // 叶节点
        return std::make_unique<CGHTLeafNode>(data, distanceType, dataType, method, 1);
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 选择三个pivots
    std::vector<int> pivotIndices = PivotSelector::selectPivots(
        data,
        3,              // 改为选择3个pivots
        dist,
        method,
        0.35
    );

    // 提取三个pivots
    DataPtr pivot1 = data[pivotIndices[0]];
    DataPtr pivot2 = data[pivotIndices[1]];
    DataPtr pivot3 = data[pivotIndices[2]];

    // 计算所有点到三个pivots的距离
    std::vector<std::tuple<DataPtr, long double, long double, long double>> distances;
    for (const auto& item : data) {
        long double d1 = dist->distance(*pivot1, *item);
        long double d2 = dist->distance(*pivot2, *item);
        long double d3 = dist->distance(*pivot3, *item);
        distances.push_back({ item, d1, d2, d3 });
    }

    // 基于三个特征进行三维聚类分区
    auto partitions = clusterPartition(distances, f);

    // 计算每个partition的三个特征值的范围
    std::vector<std::pair<long double, long double>> feature1Ranges; // d1+d2-d3范围
    std::vector<std::pair<long double, long double>> feature2Ranges; // d1-d2+d3范围
    std::vector<std::pair<long double, long double>> feature3Ranges; // d1-d2-d3范围

    for (const auto& partition : partitions) {
        if (partition.empty()) {
            feature1Ranges.emplace_back(0, 0);
            feature2Ranges.emplace_back(0, 0);
            feature3Ranges.emplace_back(0, 0);
            continue;
        }

        long double f1Min = std::numeric_limits<long double>::infinity();
        long double f1Max = -std::numeric_limits<long double>::infinity();
        long double f2Min = std::numeric_limits<long double>::infinity();
        long double f2Max = -std::numeric_limits<long double>::infinity();
        long double f3Min = std::numeric_limits<long double>::infinity();
        long double f3Max = -std::numeric_limits<long double>::infinity();

        for (const auto& entry : partition) {
            auto [item, d1, d2, d3] = entry;
            long double f1 = d1 + d2 - d3;  // d1+d2-d3
            long double f2 = d1 - d2 + d3;  // d1-d2+d3
            long double f3 = d1 - d2 - d3;  // d1-d2-d3

            f1Min = std::min(f1Min, f1);
            f1Max = std::max(f1Max, f1);
            f2Min = std::min(f2Min, f2);
            f2Max = std::max(f2Max, f2);
            f3Min = std::min(f3Min, f3);
            f3Max = std::max(f3Max, f3);
        }

        feature1Ranges.emplace_back(f1Min, f1Max);
        feature2Ranges.emplace_back(f2Min, f2Max);
        feature3Ranges.emplace_back(f3Min, f3Max);
    }

    // 递归构建子树
    std::vector<std::unique_ptr<CGHTNode>> children;
    for (auto& partition : partitions) {
        std::vector<DataPtr> partitionData;
        for (const auto& entry : partition) {
            auto [item, d1, d2, d3] = entry;
            partitionData.push_back(item);
        }
        children.push_back(bulkLoad(partitionData, distanceType, dataType, method, f));
    }

    return std::make_unique<CGHTInternalNode>(
        pivot1, pivot2, pivot3, std::move(children),
        std::move(feature1Ranges), std::move(feature2Ranges), std::move(feature3Ranges), dist);
}

std::vector<std::vector<std::tuple<DataPtr, long double, long double, long double>>>
CGHTree::clusterPartition(
    std::vector<std::tuple<DataPtr, long double, long double, long double>>& distances,
    int f /* 新增参数：每个维度划分f次 */)
{
    if (distances.empty()) {
        return {};
    }

    // 提取三个特征值
    std::vector<long double> feature1Values, feature2Values, feature3Values;
    for (const auto& entry : distances) {
        auto [item, d1, d2, d3] = entry;
        feature1Values.push_back(d1 + d2 - d3);  // d1+d2-d3
        feature2Values.push_back(d1 - d2 + d3);  // d1-d2+d3
        feature3Values.push_back(d1 - d2 - d3);  // d1-d2-d3
    }

    // 计算每个特征的f个分割点（按百分位数）
    std::sort(feature1Values.begin(), feature1Values.end());
    std::sort(feature2Values.begin(), feature2Values.end());
    std::sort(feature3Values.begin(), feature3Values.end());

    std::vector<long double> thresholds1, thresholds2, thresholds3;
    for (int i = 1; i < f; ++i) {
        size_t index = (feature1Values.size() * i) / f;
        thresholds1.push_back(feature1Values[index]);

        index = (feature2Values.size() * i) / f;
        thresholds2.push_back(feature2Values[index]);

        index = (feature3Values.size() * i) / f;
        thresholds3.push_back(feature3Values[index]);
    }

    // 创建 f^3 个分区
    int totalPartitions = f * f * f;
    std::vector<std::vector<std::tuple<DataPtr, long double, long double, long double>>> partitions(totalPartitions);

    for (const auto& entry : distances) {
        auto [item, d1, d2, d3] = entry;
        long double f1 = d1 + d2 - d3;
        long double f2 = d1 - d2 + d3;
        long double f3 = d1 - d2 - d3;

        // 计算在每个维度上的索引
        int idx1 = 0;
        while (idx1 < f - 1 && f1 > thresholds1[idx1]) ++idx1;

        int idx2 = 0;
        while (idx2 < f - 1 && f2 > thresholds2[idx2]) ++idx2;

        int idx3 = 0;
        while (idx3 < f - 1 && f3 > thresholds3[idx3]) ++idx3;

        // 计算最终分区索引（三维到一维的映射）
        int finalIdx = idx1 * f * f + idx2 * f + idx3;
        partitions[finalIdx].push_back(entry);
    }

    return partitions;
}

void CGHTree::runCGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    // 选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    int f;
    std::cout << "请输入每个维度的划分次数 f (≥2): ";
    std::cin >> f;
    if (f < 2) {
        std::cerr << "无效的 f 值（必须 ≥ 2）" << std::endl;
        return;
    }

    std::cout << "\n注意：\n"
        << "- 3D CGHT 树将在每个内部节点，从其局部数据子集中动态选择 3 个支撑点\n"
        << "- 每个维度将划分成 " << f << " 个区间\n";

    // 构建 CGHT 树
    auto treeRoot = CGHTree::bulkLoad(dataset, distanceType, dataType, method, f);

    int querySource;
    std::cout << "请选择查询点来源：\n"
        << "1 - 从现有数据集中选择\n"
        << "2 - 自定义输入新查询点\n"
        << "请输入选项编号：";
    std::cin >> querySource;

    if (std::cin.fail() || (querySource != 1 && querySource != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "输入无效，请选择 1 或 2。\n";
        return;
    }

    std::shared_ptr<MetricData> queryPtr;

    if (querySource == 1) {
        int queryIndex;
        std::cout << "请选择一个查询对象索引（0 到 " << dataset.size() - 1 << "）：";
        std::cin >> queryIndex;

        if (std::cin.fail() || queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入有效的索引范围。\n";
            return;
        }
        queryPtr = dataset[queryIndex];
    }
    else {
        try {
            queryPtr = inputCustomQuery(dataType);
        }
        catch (const std::exception& e) {
            std::cout << "自定义输入失败：" << e.what() << std::endl;
            return;
        }
    }

    long double threshold;
    std::cout << "请输入查询半径 r: ";
    std::cin >> threshold;
    if (threshold < 0) {
        std::cerr << "查询半径不能为负数。" << std::endl;
        return;
    }

    const MetricData& query = *queryPtr;
    auto results = treeRoot->rangeSearch(query, threshold, &CGHTree::distanceCalculations_);

    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    std::cout << "\n- 查询对象 #" << queryPtr << ": " << queryPtr->toString() << std::endl;
    std::cout << "\n找到匹配项数量（不包括查询对象自身）: " << filteredResults.size() << std::endl;

    if (!filteredResults.empty()) {
        std::cout << "以下是匹配项：" << std::endl;
        for (size_t i = 0; i < filteredResults.size(); ++i) {
            std::cout << "  - 匹配项 #" << i + 1 << ": " << filteredResults[i]->toString() << std::endl;
        }
    }
    else {
        std::cout << "未找到任何匹配项。" << std::endl;
    }

    std::cout << "\n本次查询共调用距离函数: " << CGHTree::getDistanceCalculations() << " 次" << std::endl;
}

