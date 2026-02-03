
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTree.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTInternalNode.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTree.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>

const int MaxLeafSize = 20;

long long MVPTree::distanceCalculations_ = 0;

//  修改：移除 selectedPivots，增加 method 参数
std::unique_ptr<MVPTNode> MVPTree::bulkLoad(
    const std::vector<DataPtr>& data,
    int k, int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method)  // ← 新增 method
{
    if (data.size() <= MaxLeafSize) {
        // 叶节点：可以构建 PivotTable（如果你实现了），或直接存数据
        return std::make_unique<MVPTLeafNode>(data, distanceType, dataType, method);
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    //  关键修改：在当前 data 子集上动态选择 k 个支撑点
    std::vector<int> pivotIndices = PivotSelector::selectPivots(
        data,           // 当前子集
        k,              // 用户指定的 k
        dist,
        method,         // 用户选择的算法（MaxVar / FFT / Random 等）
        0.35            // alpha（仅部分方法使用）
    );

    // 提取支撑点（vantage points）
    std::vector<DataPtr> vps;
    for (int idx : pivotIndices) {
        vps.push_back(data[idx]);
    }

    // 构建 restData：排除支撑点
    std::vector<DataPtr> restData;
    for (const auto& item : data) {
        bool isVP = false;
        for (const auto& vp : vps) {
            if (item == vp) {
                isVP = true;
                break;
            }
        }
        if (!isVP) {
            restData.push_back(item);
        }
    }

    // 划分数据：逐层按每个 VP 分成 f 份
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

    // 计算每个 partition 对每个 VP 的距离上下界
    std::vector<std::vector<long double>> lowerBounds(vps.size(), std::vector<long double>(partitions.size()));
    std::vector<std::vector<long double>> upperBounds(vps.size(), std::vector<long double>(partitions.size()));
    computeBounds(vps, partitions, *dist, lowerBounds, upperBounds);

    // 递归构建子树（传递相同的 method）
    std::vector<std::unique_ptr<MVPTNode>> children;
    for (auto& part : partitions) {
        children.push_back(bulkLoad(part, k, f, distanceType, dataType, method)); // ← 递归传 method
    }

    return std::make_unique<MVPTInternalNode>(
        vps, std::move(children), std::move(lowerBounds), std::move(upperBounds), dist);
}

//  删除 selectKVPs 函数（不再需要）

// splitByRadius 和 computeBounds 保持不变（逻辑正确）
//此处实现为距离平衡划分！
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
            long double minD = std::numeric_limits<long double>::infinity();
            long double maxD = -std::numeric_limits<long double>::infinity();
            for (const auto& item : partitions[ci]) {
                long double d = dist.distance(*vpList[pi], *item);
                if (d < minD) minD = d;
                if (d > maxD) maxD = d;
            }
            lowerBounds[pi][ci] = minD;
            upperBounds[pi][ci] = maxD;
        }
    }
}

// 修改 runMVPTRangeSearch：不再传 selectedPivots，而是传 method
void MVPTree::runMVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    int k;
    std::cout << "请输入支撑点数量 k (≥1): ";
    std::cin >> k;
    if (k < 1 || k > static_cast<int>(dataset.size())) {
        std::cerr << "无效的支撑点数量 k（必须 ≥ 1 且 ≤ 数据集大小）" << std::endl;
        return;
    }

    int f;
    std::cout << "请输入每个支撑点划分的子区域数量 f (≥2): ";
    std::cin >> f;
    if (f < 2) {
        std::cerr << "无效的 f 值（必须 ≥ 2）" << std::endl;
        return;
    }

    // 选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    //  不再需要 selectedPivots！也不再调用 PivotSelector 在根节点选点
    // 因为现在每个节点都会自己选！

    std::cout << "\n注意：\n"
        "- MVP 树将在每个内部节点，从其局部数据子集中动态选择 " << k << " 个支撑点\n";

    // 构建 MVP 树（传 method）
    auto treeRoot = MVPTree::bulkLoad(dataset, k, f, distanceType, dataType, method);

    // 后续查询逻辑保持不变（与 GHT 完全一致）
    // ... [用户输入 query、threshold、执行查询、输出结果] ...

    // （以下省略，你已有完整实现，只需删除 selectedPivots 相关行）
    // 我保留关键部分以确保接口一致

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

    long double threshold;
    std::cout << "请输入查询半径 r: ";
    std::cin >> threshold;
    if (threshold < 0) {
        std::cerr << "查询半径不能为负数。" << std::endl;
        return;
    }

    const MetricData& query = *queryPtr;
    auto results = treeRoot->rangeSearch(query, threshold, &MVPTree::distanceCalculations_);

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

    std::cout << "\n本次查询共调用距离函数: " << MVPTree::getDistanceCalculations() << " 次" << std::endl;
}