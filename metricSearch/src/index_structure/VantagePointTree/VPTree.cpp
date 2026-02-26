// VPTree.cpp
#include "../../../include/index_structure/VantagePointTree/VPTree.h"
#include "../../../include/index_structure/VantagePointTree/VPTInternalNode.h"
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"

#include <iostream>
#include <algorithm>
#include <random>

const int MaxLeafSize = 20;

long long VPTree::distanceCalculations_ = 0;

// 新增：从当前子集动态选择 VP 和半径
std::pair<std::shared_ptr<MetricData>, long double> VPTree::selectVPAndRadius(
    const std::vector<std::shared_ptr<MetricData>>& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
{
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 从当前 data 中选择 1 个最优 VP
    std::vector<int> indices = PivotSelector::selectPivots(data, 1, dist, method, 0.35);
    auto vp = data[indices[0]];

    long double medianRadius = getMedianRadius(data, *vp, *dist);
    return { vp, medianRadius };
}

// 取中位数作为划分半径
long double VPTree::getMedianRadius(const std::vector<std::shared_ptr<MetricData>>& data,
    const MetricData& vp,
    const MetricDistance& dist) {
    std::vector<long double> distances;
    for (const auto& item : data) {
        if (item.get() != &vp) {
            distances.push_back(dist.distance(vp, *item));
        }
    }

    if (distances.empty()) return 0.0L;

    std::sort(distances.begin(), distances.end());
    return distances[distances.size() / 2];
}

// 批量构建 VPT 树：新增 method 参数
std::unique_ptr<VPTNode> VPTree::bulkLoad(
    const std::vector<DataPtr>& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
{
    if (data.size() <= MaxLeafSize) {
        // 叶子节点：也从当前 data 动态选 1 个 pivot
        return std::make_unique<VPTLeafNode>(data, distanceType, dataType, method);
    }

    // 从当前子集选择 VP
    auto [vp, radius] = selectVPAndRadius(data, distanceType, dataType, method);

    DataList leftData, rightData;
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    for (const auto& item : data) {
        if (item == vp) continue;
        long double d = dist->distance(*item, *vp);
        if (d <= radius)
            leftData.push_back(item);
        else
            rightData.push_back(item);
    }

    // 递归构建子树，传递相同的 method
    auto left = bulkLoad(leftData, distanceType, dataType, method);
    auto right = bulkLoad(rightData, distanceType, dataType, method);

    return std::make_unique<VPTInternalNode>(vp, radius, std::move(left), std::move(right), dist);
}

void VPTree::runVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    std::cout << "\n注意：VPT 每次分裂仅使用 1 个支撑点（VP）。\n"
        << "您选择的算法将用于在每个子集中动态选择该点。\n";

    // 选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    // 构建 VPT 树（不再需要全局 selectedPivots）
    auto treeRoot = VPTree::bulkLoad(dataset, distanceType, dataType, method);

    // 用户选择查询点来源
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

    // 获取查询半径
    long double threshold;
    std::cout << "请输入查询半径 r: ";
    std::cin >> threshold;

    if (threshold < 0) {
        std::cerr << "查询半径不能为负数。" << std::endl;
        return;
    }

    const MetricData& query = *queryPtr;
    VPTree::resetDistanceCalculations();

    auto results = treeRoot->rangeSearch(query, threshold, &VPTree::distanceCalculations_);

    // 过滤掉查询对象本身
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // 输出结果
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

    std::cout << "\n本次查询共调用距离函数: " << VPTree::getDistanceCalculations() << " 次" << std::endl;
}