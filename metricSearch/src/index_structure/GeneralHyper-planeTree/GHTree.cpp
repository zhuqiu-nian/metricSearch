#include "../../../include/index_structure/GeneralHyper-planeTree/GHTree.h"
#include "../../../include/utils/MetricSpaceSearch.h"  // 包含 createDistanceFunction
#include "../../../include/index_structure/GeneralHyper-planeTree/GHTInternalNode.h"
#include "../../../include/index_structure/GeneralHyper-planeTree/GHTLeafNode.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>

long long GHTree::distanceCalculations_ = 0;

// 随机选择两个支撑点
std::pair<DataPtr, DataPtr> GHTree::selectPivots(const DataList& data) {
    std::vector<int> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    return { data[indices[0]], data[indices[1]] };
}

// 批量构建 GHT 树
std::unique_ptr<GHTNode> GHTree::bulkLoad(
    const DataList& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
{
    const int MaxLeafSize = 20;

    if (data.size() <= MaxLeafSize) {
        return std::make_unique<GHTLeafNode>(data, distanceType, dataType, method);
    }

    // 创建距离函数（每次递归都相同，可考虑提升到外层传入以优化）
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    //关键：在当前 data 子集上选择 2 个支撑点
    std::vector<int> pivotIndices = PivotSelector::selectPivots(
        data,           // 当前子集
        2,              // GHT 必须为 2
        dist,
        method,         // 用户选择的算法
        0.35            // alpha 参数（仅某些方法使用，可设默认值）
    );

    DataPtr c1 = data[pivotIndices[0]];
    DataPtr c2 = data[pivotIndices[1]];

    // 划分数据
    DataList leftData, rightData;
    for (const auto& item : data) {
        if (item == c1 || item == c2) continue;
        long double d1 = dist->distance(*item, *c1);
        long double d2 = dist->distance(*item, *c2);
        if (d1 <= d2) {
            leftData.push_back(item);
        }
        else {
            rightData.push_back(item);
        }
    }

    // 递归构建子树（传递相同的 method）
    auto left = bulkLoad(leftData, distanceType, dataType, method);
    auto right = bulkLoad(rightData, distanceType, dataType, method);

    return std::make_unique<GHTInternalNode>(c1, c2, std::move(left), std::move(right), dist);
}

void GHTree::runGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    //输入支撑点个数
    std::cout << "\n注意：GHT 每次分裂仅使用 2 个支撑点。\n"
        "您选择的算法将用于在每个子集中动态选择这 2 个点。\n";

    //选择支撑点选择算法
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    //创建距离函数
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);  // 创建距离函数

    // 构建 GHT 树
    auto treeRoot = GHTree::bulkLoad(dataset, distanceType, dataType, method);

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

    // 执行范围查询
    auto results = treeRoot->rangeSearch(query, threshold, &GHTree::distanceCalculations_);

    // 过滤掉查询对象本身
    //注意，此处的query定义与Pivottable略有区别，所以此处写法不同
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
    std::cout << "\n本次查询共调用距离函数: " << GHTree::getDistanceCalculations() << " 次" << std::endl;
}