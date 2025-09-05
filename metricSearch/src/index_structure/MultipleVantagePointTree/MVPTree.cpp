// MVPTree.cpp
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTree.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTInternalNode.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"

#include <algorithm>
#include <iostream>

const int MaxLeafSize = 20;

long long MVPTree::distanceCalculations_ = 0;

std::unique_ptr<MVPTNode> MVPTree::bulkLoad(const std::vector<DataPtr>& data,
    int k, int f,
    int distanceType, int dataType,
    std::vector<int> selectedPivots) {
    if (data.size() <= MaxLeafSize) {
        return std::make_unique<MVPTLeafNode>(data, distanceType, dataType, selectedPivots);
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    std::vector<DataPtr> vps = selectKVPs(data, k, distanceType, dataType);
    std::vector<DataPtr> restData = data;
    for (const auto& vp : vps) {
        restData.erase(std::remove(restData.begin(), restData.end(), vp), restData.end());
    }

    std::vector<std::vector<DataPtr>> partitions = { restData };

    for (int i = 0; i < k; ++i) {
        std::vector<std::vector<DataPtr>> newPartitions;
        const auto& vp = vps[i];

        for (const auto& part : partitions) {
            auto subParts = splitByRadius(part, vp, f, *dist);
            newPartitions.insert(newPartitions.end(), subParts.begin(), subParts.end());
        }
        partitions = newPartitions;
    }

    std::vector<std::vector<long double>> lowerBounds(vps.size(), std::vector<long double>(partitions.size()));
    std::vector<std::vector<long double>> upperBounds(vps.size(), std::vector<long double>(partitions.size()));

    computeBounds(vps, partitions, *dist, lowerBounds, upperBounds);

    std::vector<std::unique_ptr<MVPTNode>> children;
    for (auto& part : partitions) {
        children.push_back(bulkLoad(part, k, f, distanceType, dataType, selectedPivots));
    }

    return std::make_unique<MVPTInternalNode>(
        vps, std::move(children), std::move(lowerBounds), std::move(upperBounds), dist);
}

std::vector<DataPtr> MVPTree::selectKVPs(const std::vector<DataPtr>& data, int k,
    int distanceType, int dataType) {
    std::vector<DataPtr> vps;
    for (int i = 0; i < k && i < data.size(); ++i) {
        vps.push_back(data[i]);
    }
    return vps;
}

std::vector<std::vector<DataPtr>> MVPTree::splitByRadius(
    const std::vector<DataPtr>& data, const DataPtr& vp,
    int f, const MetricDistance& dist) {

    std::vector<std::vector<DataPtr>> parts(f);
    if (data.empty()) return parts;

    std::vector<long double> distances;
    for (const auto& item : data) {
        distances.push_back(dist.distance(*vp, *item));
    }

    std::sort(distances.begin(), distances.end());
    std::vector<long double> thresholds;
    for (int i = 1; i < f; ++i) {
        size_t index = (distances.size() * i) / f;
        thresholds.push_back(distances[index]);
    }

    for (const auto& item : data) {
        long double d = dist.distance(*vp, *item);
        int idx = 0;
        while (idx < f - 1 && d > thresholds[idx]) ++idx;
        parts[idx].push_back(item);
    }

    return parts;
}

void MVPTree::computeBounds(
    const std::vector<DataPtr>& vpList,
    const std::vector<std::vector<DataPtr>>& partitions,
    const MetricDistance& dist,
    std::vector<std::vector<long double>>& lowerBounds,
    std::vector<std::vector<long double>>& upperBounds) {

    for (size_t pi = 0; pi < vpList.size(); ++pi) {
        for (size_t ci = 0; ci < partitions.size(); ++ci) {
            long double minD = INFINITY, maxD = -INFINITY;
            for (const auto& item : partitions[ci]) {
                long double d = dist.distance(*vpList[pi], *item);
                minD = std::min(minD, d);
                maxD = std::max(maxD, d);
            }
            lowerBounds[pi][ci] = minD;
            upperBounds[pi][ci] = maxD;
        }
    }
}

void MVPTree::runMVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    // 获取用户输入：支撑点数量 k
    int k;
    std::cout << "请输入支撑点数量 k (≥1): ";
    std::cin >> k;

    if (k < 1 || k > static_cast<int>(dataset.size())) {
        std::cerr << "无效的支撑点数量 k（必须 ≥ 1 且 ≤ 数据集大小）" << std::endl;
        return;
    }

    // 获取用户输入：每个支撑点划分的子区域数量 f
    int f;
    std::cout << "请输入每个支撑点划分的子区域数量 f (≥2): ";
    std::cin >> f;

    if (f < 2) {
        std::cerr << "无效的 f 值（必须 ≥ 2）" << std::endl;
        return;
    }

    //选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    //创建距离函数
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);  // 创建距离函数

    //调用 PivotSelector 生成支撑点索引
    double alpha = 0.35; // 稀疏空间法参数
    std::vector<int> selectedPivots = PivotSelector::selectPivots(
        dataset,
        k,
        dist,
        method,
        alpha
    );

    // 构建 MVP 树
    auto treeRoot = MVPTree::bulkLoad(dataset, k, f, distanceType, dataType, selectedPivots);

    // 用户选择查询点来源
    int querySource;
    cout << "请选择查询点来源：\n"
        << "1 - 从现有数据集中选择\n"
        << "2 - 自定义输入新查询点\n"
        << "请输入选项编号：";
    cin >> querySource;

    if (cin.fail() || (querySource != 1 && querySource != 2)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请选择 1 或 2。\n";
        return;
    }

    shared_ptr<MetricData> queryPtr;

    if (querySource == 1) {
        int queryIndex;
        cout << "请选择一个查询对象索引（0 到 " << dataset.size() - 1 << "）：";
        cin >> queryIndex;

        if (cin.fail() || queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请输入有效的索引范围。\n";
            return;
        }
        queryPtr = dataset[queryIndex];
    }
    else {
        try {
            queryPtr = inputCustomQuery(dataType);
        }
        catch (const exception& e) {
            cout << "自定义输入失败：" << e.what() << endl;
            return;
        }
    }

=======
    // 获取用户输入：查询对象索引
    int queryIndex;
    std::cout << "请选择查询对象索引 (0-" << dataset.size() - 1 << "): ";
    std::cin >> queryIndex;

    if (queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
        std::cerr << "无效的查询对象索引。" << std::endl;
        return;
    }

    // 获取用户输入：查询半径 r
    long double threshold;
    std::cout << "请输入查询半径 r: ";
    std::cin >> threshold;

    if (threshold < 0) {
        std::cerr << "查询半径不能为负数。" << std::endl;
        return;
    }

    // 获取查询对象

    const MetricData& query = *queryPtr;

    // 执行范围查询
    auto results = treeRoot->rangeSearch(query, threshold, &MVPTree::distanceCalculations_);

    // 过滤掉查询对象本身
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // 输出当前查询对象信息
    std::cout << "\n- 查询对象 #" << queryPtr << ": " << queryPtr->toString() << std::endl;

    // 输出结果
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
    // 输出距离计算次数
    std::cout << "\n本次查询共调用距离函数: " << MVPTree::getDistanceCalculations() << " 次" << std::endl;
}