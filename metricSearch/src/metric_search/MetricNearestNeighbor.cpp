#include "../../include/metric_search/MetricNearestNeighbor.h"
#include "../../include/utils/Solution.h"
#include <iostream>
#include <unordered_map>
#include <limits>
#include <cmath>

using namespace std;

MetricSpaceSearch::SearchResult MetricSpaceNNQuery::nearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    const shared_ptr<MetricData>& pivot)
{
    const long double EPS = 1e-15;
    MetricSpaceSearch::SearchResult result;
    result.pivot = pivot;
    result.distanceFunc = distanceFunc;
    result.calculations = 0;

    if (dataset.empty()) {
        return result; // 数据集为空，直接返回空结果
    }

    // 如果未指定支撑点，则默认使用第一个数据点作为支撑点
    shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // d(p, q)：支撑点到查询点的距离（优先从缓存中获取）
    long double d_pq;
    if (MetricSpaceSearch::pivotDistanceCache.count(initialPivot.get()) &&
        MetricSpaceSearch::pivotDistanceCache[initialPivot.get()].count(query.get()))
    {
        d_pq = MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else
    {
        cout << "\n----------------[DEBUG]:预处理不完全----------------\n";
        d_pq = distanceFunc->distance(*initialPivot, *query);
        MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][query.get()] = d_pq;
    }

    // 初始化最近邻为无穷大和空指针
    result.distance = numeric_limits<long double>::max();
    result.nearest.reset();

    // === 直接选择第一个非支撑点、非查询点作为初始最近邻 ===
    bool foundInitial = false;
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query) {
            continue; // 跳过支撑点和查询点
        }

        // 找到第一个符合条件的点
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;
        result.distance = d_qx;
        result.nearest = data;
        result.steps.push_back("初始化最近邻为: " + data->toString() +
            "，初始距离为: " + to_string(d_qx));
        foundInitial = true;
        break;
    }

    if (!foundInitial) {
        result.steps.push_back("警告：没有找到可用的初始最近邻！");
        return result;
    }

    // === 开始对整个数据集进行搜索 ===
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query || data == result.nearest) {
            continue; // 跳过支撑点、查询点和当前最近邻
        }

        // 获取 d(p, x)，即支撑点到当前数据点的距离
        long double d_px;
        if (MetricSpaceSearch::pivotDistanceCache.count(initialPivot.get()) &&
            MetricSpaceSearch::pivotDistanceCache[initialPivot.get()].count(data.get()))
        {
            d_px = MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][data.get()];
        }
        else
        {
            d_px = distanceFunc->distance(*initialPivot, *data);
            MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][data.get()] = d_px;
        }

        // ====== 使用三角不等式进行剪枝 ======
        long double lower_bound = abs(d_pq - d_px);   // 下界
        long double upper_bound = d_pq + d_px;        // 上界

        // Debug 输出剪枝信息
        result.steps.push_back("[DEBUG] 当前点: " + data->toString() +
            ", d_px=" + to_string(d_px) +
            ", lower_bound=" + to_string(lower_bound) +
            ", upper_bound=" + to_string(upper_bound) +
            ", current_min=" + to_string(result.distance));

        // 上界剪枝：如果上界小于当前最小距离，跳过
        if (upper_bound < result.distance - EPS) {
            result.steps.push_back("由于上界剪枝跳过: " + data->toString());
            continue;
        }

        // 下界剪枝：如果下界大于等于当前最小距离，跳过
        if (lower_bound >= result.distance + EPS) {
            result.steps.push_back("由于下界剪枝跳过: " + data->toString());
            continue;
        }

        // 无法剪枝，必须计算真实距离
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;

        result.steps.push_back("计算查询点到 " + data->toString() +
            " 的实际距离: " + to_string(d_qx));

        // 更新最近邻
        if (d_qx < result.distance) {
            result.distance = d_qx;
            result.nearest = data;
            result.steps.push_back("发现新的最近邻: " + data->toString() +
                " 最近距离为 " + to_string(d_qx));
        }
    }

    //最后判断支撑点是不是最近邻，因为之前跳过了支撑点
    if (d_pq < result.distance) {
        result.distance = d_pq;
        result.nearest = initialPivot;
        result.steps.push_back("发现新的最近邻: 是支撑点！最近距离为 " + to_string(d_pq));
    }

    return result;
}


void runNearestNeighborQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        cout << "错误：数据集为空。\n";
        return;
    }

    char showDataChoice;
    cout << "是否展示已读入的数据集？(y/n): ";
    cin >> showDataChoice;

    if (showDataChoice == 'y' || showDataChoice == 'Y') {
        cout << "\n--- 已读入的数据集 ---\n";
        for (const auto& item : dataset) {
            cout << item->toString() << endl;
        }
        cout << "------------------------\n\n";
    }

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

    // 用户选择支撑点
    int pivotIndex;
    cout << "请选择一个支撑点索引（0 到 " << dataset.size() - 1 << "）：";
    cin >> pivotIndex;

    if (cin.fail() || pivotIndex < 0 || pivotIndex >= static_cast<int>(dataset.size())) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请输入有效的索引范围。\n";
        return;
    }

    auto pivot = dataset[pivotIndex];

    // 创建对应的距离函数
    shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 调用 nearestNeighbor 进行搜索
    cout << "\n开始最近邻搜索...\n";

    MetricSpaceSearch::SearchResult result = MetricSpaceNNQuery::nearestNeighbor(dataset, queryPtr, distanceFunc, pivot);

    // 使用统一打印函数输出结果
    printSearchResult(result);
}