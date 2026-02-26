// CLPTree.cpp
#include "../../../include/index_structure/CLPTree/CLPTree.h"
#include "../../../include/index_structure/CLPTree/CLPTInternalNode.h"
#include "../../../include/index_structure/CLPTree/CLPTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"
#include "utils/CLPPartitioner.h"

#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <limits>

const int MaxLeafSize = 20;

// 初始化静态成员
long long CLPTree::buildDistanceCalculations_ = 0;
long long CLPTree::queryDistanceCalculations_ = 0;

// ==================== 局部辅助结构体：用于投影排序 ====================
struct ProjectionIndex {
    double projection;
    size_t originalIndex;
    ProjectionIndex(double proj, size_t idx) : projection(proj), originalIndex(idx) {}
};

// ==================== 递归构建函数 ====================
std::unique_ptr<CLPTNode> CLPTree::bulkLoadRecursive(
    const DataList& data,
    const std::vector<DataPtr>& globalPivots,
    const std::vector<std::vector<double>>& normalVectors,
    int currentNormalIndex,
    int f,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method,
    MetricDistance* dist,
    long long* buildDistCount)
{
    // 叶节点条件
    if (currentNormalIndex >= static_cast<int>(normalVectors.size()) ||
        data.size() <= static_cast<size_t>(MaxLeafSize)) {
        return std::make_unique<CLPTLeafNode>(data, distanceType, dataType, method);
    }

    // 获取当前法向量
    const auto& currentNormal = normalVectors[currentNormalIndex];

    // 计算数据在支撑点空间中的坐标
    auto dataCoordinates = CLPPartitioner::getCordinate(dist, globalPivots, data, buildDistCount);

    // 单层划分（获取边界）
    auto partitionResult = CLPPartitioner::partitionSingleLevel(dataCoordinates, currentNormal, f);

    // 根据投影重新组织数据到 f 个子集
    std::vector<std::vector<size_t>> childIndices(f);
    std::vector<ProjectionIndex> projections;
    for (size_t i = 0; i < data.size(); ++i) {
        double proj = 0.0;
        for (size_t d = 0; d < currentNormal.size(); ++d) {
            proj += dataCoordinates[i][d] * currentNormal[d];
        }
        projections.emplace_back(proj, i);
    }

    std::sort(projections.begin(), projections.end(),
        [](const ProjectionIndex& a, const ProjectionIndex& b) {
            return a.projection < b.projection;
        });

    size_t bucketSize = (data.size() + f - 1) / f; // 向上取整
    for (int child = 0; child < f; ++child) {
        size_t start = child * bucketSize;
        size_t end = std::min(start + bucketSize, data.size());
        for (size_t j = start; j < end; ++j) {
            childIndices[child].push_back(projections[j].originalIndex);
        }
    }

    // 构建子数据集
    std::vector<DataList> childDataList(f);
    for (int child = 0; child < f; ++child) {
        for (size_t idx : childIndices[child]) {
            childDataList[child].push_back(data[idx]);
        }
    }

    // 递归构建子树
    std::vector<std::unique_ptr<CLPTNode>> children;
    for (int i = 0; i < f; ++i) {
        if (!childDataList[i].empty()) {
            auto child = bulkLoadRecursive(
                childDataList[i], globalPivots, normalVectors,
                currentNormalIndex + 1, f, distanceType, dataType, method,
                dist, buildDistCount);
            children.push_back(std::move(child));
        }
    }

    // 计算 longestDistancesToPivots（用于 canInclude，可选）
    std::vector<std::vector<double>> longestDistances;
    for (int i = 0; i < static_cast<int>(children.size()); ++i) {
        std::vector<double> farthest(globalPivots.size(), 0.0);
        for (const auto& item : childDataList[i]) {
            for (size_t p = 0; p < globalPivots.size(); ++p) {
                double d = dist->distance(*item, *globalPivots[p]);
                if (buildDistCount) (*buildDistCount)++;
                if (d > farthest[p]) farthest[p] = d;
            }
        }
        longestDistances.push_back(farthest);
    }

    // 创建内部节点（只包含当前层的法向量和边界）
    std::vector<std::vector<double>> nodeNormals = { currentNormal };
    // 注意：partitionResult.lowerBounds/upperBounds 是 vector<double>，每个元素对应一个子区域
    // 我们需要为每个非空子节点提取对应的边界
    std::vector<std::vector<double>> nodeLower;
    std::vector<std::vector<double>> nodeUpper;

    int childIdx = 0;
    for (int i = 0; i < f; ++i) {
        if (!childDataList[i].empty()) {
            nodeLower.push_back({ partitionResult.lowerBounds[i] });
            nodeUpper.push_back({ partitionResult.upperBounds[i] });
            childIdx++;
        }
    }

    auto dist_shared_ptr = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);//给下面的CLPTInternalNode用的，因为某些奇奇怪怪的原因没能兼容

    return std::make_unique<CLPTInternalNode>(
        globalPivots,
        std::move(children),
        nodeNormals,
        nodeLower,
        nodeUpper,
        longestDistances,
        dist_shared_ptr
    );
}

// ==================== 公共构建接口 ====================
std::unique_ptr<CLPTNode> CLPTree::bulkLoad(
    const DataList& data,
    int k,
    int f,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
{
    if (data.size() <= static_cast<size_t>(MaxLeafSize)) {
        return std::make_unique<CLPTLeafNode>(data, distanceType, dataType, method);
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // Step 1: 选择全局支撑点（仅根节点）
    std::vector<int> pivotIndices = PivotSelector::selectPivots(data, k, dist, method, 0.35);
    std::vector<DataPtr> globalPivots;
    for (int idx : pivotIndices) {
        globalPivots.push_back(data[idx]);
    }

    // Step 2: 获取法向量（使用 CGH：标准基向量，确保线性无关）
    auto normalVectors = CLPPartitioner::getCGHNormalVectors(k);

    // Step 3: 递归构建
    return bulkLoadRecursive(
        data, globalPivots, normalVectors, 0, f, distanceType, dataType, method,
        dist.get(), &buildDistanceCalculations_);
}

// ==================== 查询入口 ====================
void CLPTree::runCLPTRangeSearch(const DataList& dataset, int distanceType, int dataType) {
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    int k, f;
    std::cout << "请输入支撑点数量 k (≥1): ";
    std::cin >> k;
    if (k < 1 || k > static_cast<int>(dataset.size())) {
        std::cerr << "无效的支撑点数量 k（必须 ≥ 1 且 ≤ 数据集大小）" << std::endl;
        return;
    }

    std::cout << "请输入每个支撑点划分的子区域数量 f (≥2): ";
    std::cin >> f;
    if (f < 2) {
        std::cerr << "无效的 f 值（必须 ≥ 2）" << std::endl;
        return;
    }

    // 选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    std::cout << "\n开始构建 CLP 树...\n";
    auto treeRoot = CLPTree::bulkLoad(dataset, k, f, distanceType, dataType, method);

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

    DataPtr queryPtr;
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
            queryPtr = inputCustomQuery(dataType); // 假设 inputCustomQuery 已定义
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
    CLPTree::resetDistanceCalculations(); // 重置查询计数器
    auto results = treeRoot->rangeSearch(query, threshold, &CLPTree::queryDistanceCalculations_);

    std::vector<DataPtr> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) { // 过滤掉查询点本身
            filteredResults.push_back(item);
        }
    }

    std::cout << "\n- 查询对象: " << queryPtr->toString() << std::endl;
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

    std::cout << "\n本次查询共调用距离函数: " << CLPTree::getQueryDistanceCalculations() << " 次" << std::endl;
    std::cout << "构建阶段调用距离函数: " << CLPTree::getBuildDistanceCalculations() << " 次" << std::endl;
}