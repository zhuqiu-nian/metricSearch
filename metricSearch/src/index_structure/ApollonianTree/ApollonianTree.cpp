// ApollonianTree.cpp
#include "../../../include/index_structure/ApollonianTree/ApollonianTree.h"
#include "../../../include/index_structure/ApollonianTree/ApollonianInternalNode.h"
#include "../../../include/index_structure/ApollonianTree/ApollonianLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../PivotSelector/PivotSelector.h"
#include "../../../include/utils/Solution.h"
#include <iostream>
#include <algorithm>
#include <random>

const int MaxLeafSize = 20;
long long ApollonianTree::distanceCalculations_ = 0;

// 新增：根据当前数据和两个枢轴，自动计算最优的 c1_ratio 和 c2_ratio
std::pair<long double, long double> ApollonianTree::computeOptimalRatios(
    const std::vector<DataPtr>& data,
    const DataPtr& c1,
    const DataPtr& c2,
    const MetricDistance& dist)
{
    if (data.empty()) {
        // 若无数据，返回默认安全值（不会被使用）
        return { 0.5L, 2.0L };
    }

    std::vector<long double> ratios;
    ratios.reserve(data.size());

    for (const auto& x : data) {
        long double d1 = dist.distance(*c1, *x);
        long double d2 = dist.distance(*c2, *x);

        if (d2 == 0) {
            // x == c2 → ratio = +∞，用一个极大值表示
            ratios.push_back(std::numeric_limits<long double>::max());
        }
        else {
            ratios.push_back(d1 / d2);
        }
    }

    // 排序以计算分位数
    std::sort(ratios.begin(), ratios.end());

    size_t n = ratios.size();
    if (n == 1) {
        // 只有一个点，无法划分，返回包围它的区间
        long double r = ratios[0];
        if (r == 0) {
            return { 0.5L, 2.0L }; // fallback
        }
        else if (r == std::numeric_limits<long double>::max()) {
            return { 0.5L, 2.0L };
        }
        else {
            return { r * 0.9L, r * 1.1L };
        }
    }

    // 使用 1/3 和 2/3 分位数（近似三等分）
    size_t idx1 = n / 3;
    size_t idx2 = (2 * n) / 3;

    long double c1_ratio = ratios[idx1];
    long double c2_ratio = ratios[idx2];

    // 安全处理：确保 c1_ratio > 0 且 c2_ratio > c1_ratio
    if (c1_ratio <= 0) c1_ratio = std::numeric_limits<long double>::min();
    if (c2_ratio <= c1_ratio) {
        if (c1_ratio < 1.0L) {
            c2_ratio = c1_ratio * 2.0L;
        }
        else {
            c2_ratio = c1_ratio + 1.0L;
        }
    }

    return { c1_ratio, c2_ratio };
}

std::vector<std::vector<DataPtr>> ApollonianTree::splitIntoThree(
    const std::vector<DataPtr>& data,
    const DataPtr& c1,
    const DataPtr& c2,
    long double c1_ratio,
    long double c2_ratio,
    const MetricDistance& dist)
{
    std::vector<DataPtr> left, mid, right;
    for (const auto& x : data) {
        long double d1 = dist.distance(*c1, *x);
        long double d2 = dist.distance(*c2, *x);
        // Avoid division by zero
        if (d2 == 0) {
            // x == c2 → assign to right
            right.push_back(x);
        }
        else {
            long double ratio = d1 / d2;
            if (ratio < c1_ratio) {
                left.push_back(x);
            }
            else if (ratio > c2_ratio) {
                right.push_back(x);
            }
            else {
                mid.push_back(x);
            }
        }
    }
    return { left, mid, right };
}

// 修改函数签名：不再需要 c1_ratio, c2_ratio
std::unique_ptr<ApollonianNode> ApollonianTree::bulkLoad(
    const std::vector<DataPtr>& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
{
    if (data.size() <= MaxLeafSize) {
        return std::make_unique<ApollonianLeafNode>(data, distanceType, dataType);
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 选择 2 个枢轴
    std::vector<int> indices = PivotSelector::selectPivots(data, 2, dist, method, 0.35);
    DataPtr c1 = data[indices[0]];
    DataPtr c2 = data[indices[1]];

    // 构建 restData（排除 c1, c2）
    std::vector<DataPtr> restData;
    for (const auto& x : data) {
        if (x != c1 && x != c2) {
            restData.push_back(x);
        }
    }

    //自动计算最优划分比例
    auto [c1_ratio, c2_ratio] = computeOptimalRatios(restData, c1, c2, *dist);

    // 按比值划分
    auto parts = splitIntoThree(restData, c1, c2, c1_ratio, c2_ratio, *dist);

    // 递归构建子树
    auto leftChild = bulkLoad(parts[0], distanceType, dataType, method);
    auto midChild = bulkLoad(parts[1], distanceType, dataType, method);
    auto rightChild = bulkLoad(parts[2], distanceType, dataType, method);

    return std::make_unique<ApollonianInternalNode>(
        c1, c2, c1_ratio, c2_ratio,
        std::move(leftChild),
        std::move(midChild),
        std::move(rightChild),
        std::move(dist)  // 注意：这里建议 move，避免拷贝
        );
}
// ApollonianTree.cpp 中的函数实现
void ApollonianTree::runApollonianRangeSearch(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    // 检查数据集是否为空
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    // 输入阿波罗尼斯划分参数 c1 和 c2
    long double c1, c2;
    std::cout << "请输入阿波罗尼斯左边界比例 c1（0 < c1 < 1，例如 0.5）：";
    std::cin >> c1;
    std::cout << "请输入阿波罗尼斯右边界比例 c2（c2 > 1，例如 2.0）：";
    std::cin >> c2;

    // 验证参数合法性
    if (c1 <= 0 || c1 >= 1 || c2 <= 1) {
        std::cerr << "错误：c1 必须满足 0 < c1 < 1，c2 必须满足 c2 > 1。" << std::endl;
        return;
    }

    // 选择支撑点选取策略（如 MaxVar / FFT / Random 等）
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    // 提示用户：每个内部节点将动态选择两个支撑点
    std::cout << "\n注意：\n"
        << "- 阿波罗尼斯树（AT）将在每个内部节点，从其局部数据子集中动态选择 2 个支撑点（C1, C2）\n"
        << "- 数据将根据比值 d(x,C1)/d(x,C2) 被划分为三部分：左子树（< c1）、中子树（[c1, c2]）、右子树（> c2）\n";

    // 构建阿波罗尼斯树
    auto treeRoot = ApollonianTree::bulkLoad(dataset, distanceType, dataType, method);

    // ========== 查询点输入逻辑（与 MVPT 完全一致）==========

    int querySource;
    std::cout << "\n请选择查询点来源：\n"
        << "1 - 从现有数据集中选择\n"
        << "2 - 自定义输入新查询点\n"
        << "请输入选项编号：";
    std::cin >> querySource;

    // 输入合法性检查
    if (std::cin.fail() || (querySource != 1 && querySource != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "输入无效，请选择 1 或 2。\n";
        return;
    }

    std::shared_ptr<MetricData> queryPtr;

    if (querySource == 1) {
        // 从数据集中选择查询点
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
        // 自定义输入查询点
        try {
            queryPtr = inputCustomQuery(dataType);
        }
        catch (const std::exception& e) {
            std::cout << "自定义输入失败：" << e.what() << std::endl;
            return;
        }
    }

    // 输入查询半径
    long double threshold;
    std::cout << "请输入查询半径 r: ";
    std::cin >> threshold;
    if (threshold < 0) {
        std::cerr << "查询半径不能为负数。" << std::endl;
        return;
    }

    // ========== 执行范围搜索 ==========

    const MetricData& query = *queryPtr;
    // 重置或使用全局距离计数器（根据你的设计）
    long long localDistanceCount = 0;
    auto results = treeRoot->rangeSearch(query, threshold, &localDistanceCount);

    // 过滤掉查询点自身（避免自匹配）
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // ========== 输出结果 ==========

    std::cout << "\n- 查询对象 #" << queryPtr << ": " << queryPtr->toString() << std::endl;
    std::cout << "\n找到匹配项数量（不包括查询对象自身）: " << filteredResults.size() << std::endl;

    if (!filteredResults.empty()) {
        std::cout << "以下是匹配项：" << std::endl;
        for (size_t i = 0; i < filteredResults.size(); ++i) {
            std::cout << "  - 匹配项 #" << (i + 1) << ": " << filteredResults[i]->toString() << std::endl;
        }
    }
    else {
        std::cout << "未找到任何匹配项。" << std::endl;
    }

    std::cout << "\n本次查询共调用距离函数: " << localDistanceCount << " 次" << std::endl;
}