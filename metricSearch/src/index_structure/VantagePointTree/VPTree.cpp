// VPTree.cpp
#include "../../../include/index_structure/VantagePointTree/VPTree.h"
#include "../../../include/index_structure/VantagePointTree/VPTInternalNode.h"
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"

#include <iostream>
#include <algorithm>

const int MaxLeafSize = 20;

long long VPTree::distanceCalculations_ = 0;

std::unique_ptr<VPTNode> VPTree::bulkLoad(const std::vector<DataPtr>& data,
    int distanceType,
    int dataType) {
    if (data.size() <= MaxLeafSize) {
        return std::make_unique<VPTLeafNode>(data, distanceType, dataType);
    }

    auto [vp, radius] = selectVPAndRadius(data, distanceType, dataType);

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

    auto left = bulkLoad(leftData, distanceType, dataType);
    auto right = bulkLoad(rightData, distanceType, dataType);

    return std::make_unique<VPTInternalNode>(vp, radius, std::move(left), std::move(right), dist);
}

//选取支撑点和划分半径，默认选择第一个点作为支撑点
std::pair<std::shared_ptr<MetricData>, long double> VPTree::selectVPAndRadius(
    const std::vector<std::shared_ptr<MetricData>>& data, int distanceType, int dataType) {

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 简单选取第一个点作为支撑点（可以优化为最大方差等）
    auto vp = data[0];

    long double medianRadius = getMedianRadius(data, *vp, *dist);

    return { vp, medianRadius };
}

//取中位数作为半径
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

void VPTree::runVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    // 构建 VP 树
    auto treeRoot = VPTree::bulkLoad(dataset, distanceType, dataType);

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

    // 重置计数器
    VPTree::resetDistanceCalculations();

    // 执行范围查询
    auto results = treeRoot->rangeSearch(query, threshold, &VPTree::distanceCalculations_);

    // 过滤掉查询对象本身
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // 输出当前查询对象信息
    std::cout << "\n- 查询对象 #" << queryPtr << ": " << queryPtr->toString() << std::endl;

    // 输出匹配项数量
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
    std::cout << "\n本次查询共调用距离函数: " << VPTree::getDistanceCalculations() << " 次" << std::endl;
}

