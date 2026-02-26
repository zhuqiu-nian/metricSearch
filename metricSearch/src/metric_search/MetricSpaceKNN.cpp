#include "../../include/metric_search/MetricSpaceKNN.h.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include "../../include/utils/Solution.h"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_set>
#include <chrono>

using namespace std;
using namespace std::chrono;

// 自定义比较器：用于维护最大堆，按距离降序排列
struct CompareDistance {
    bool operator()(const pair<shared_ptr<MetricData>, long double>& a,
        const pair<shared_ptr<MetricData>, long double>& b) {
        return a.second < b.second; // 最大堆
    }
};

MetricSpaceSearch::SearchResult knnQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    int k,
    const shared_ptr<MetricData>& pivot)
{
    MetricSpaceSearch::SearchResult result;
    result.distanceFunc = distanceFunc;
    result.k = k;
    result.calculations = 0;
    result.timeMicrosec = 0;

    if (dataset.empty() || k <= 0) {
        auto end = high_resolution_clock::now();
        result.timeMicrosec = duration_cast<microseconds>(end - high_resolution_clock::now()).count();
        return result;
    }

    auto start = high_resolution_clock::now();

    // 设置支撑点
    result.pivot = pivot ? pivot : dataset[0];
    if (!result.pivot) {
        return result;
    }

    // 获取支撑点到查询点的距离
    long double d_pq = distanceFunc->distance(*result.pivot, *query);
    result.calculations++;

    // 使用最大堆保存当前最远的 k 个邻居
    priority_queue<pair<shared_ptr<MetricData>, long double>,
        vector<pair<shared_ptr<MetricData>, long double>>,
        CompareDistance> maxHeap;

    for (const auto& data : dataset) {
        if (data == query) continue;

        // 获取支撑点到 data 的距离
        long double d_pd = 0.0L;
        auto it_pivot_data = MetricSpaceSearch::pivotDistanceCache.find(result.pivot.get());
        if (it_pivot_data != MetricSpaceSearch::pivotDistanceCache.end()) {
            auto it_data = it_pivot_data->second.find(data.get());
            if (it_data != it_pivot_data->second.end()) {
                d_pd = it_data->second; // 缓存命中
            }
            else {
                d_pd = distanceFunc->distance(*result.pivot, *data);
                result.calculations++;
                MetricSpaceSearch::pivotDistanceCache[result.pivot.get()][data.get()] = d_pd;
            }
        }
        else {
            d_pd = distanceFunc->distance(*result.pivot, *data);
            result.calculations++;
            MetricSpaceSearch::pivotDistanceCache[result.pivot.get()][data.get()] = d_pd;
        }

        // 三角不等式剪枝
        long double lower_bound = abs(d_pq - d_pd);
        long double upper_bound = d_pq + d_pd;

        if (lower_bound > result.distance) {
            result.steps.push_back("跳过 " + data->toString() + "（下界剪枝）");
            continue;
        }

        if (upper_bound <= result.distance && maxHeap.size() >= static_cast<size_t>(k)) {
            result.steps.push_back("加入 " + data->toString() + "（上界剪枝）");
        }

        // 需要实际计算 query 到 data 的距离
        long double actual_dist = distanceFunc->distance(*query, *data);
        result.calculations++;

        if (maxHeap.size() < static_cast<size_t>(k)) {
            maxHeap.push({ data, actual_dist });
            result.steps.push_back("加入 " + data->toString() + "（候选最近邻）");
        }
        else if (actual_dist < maxHeap.top().second) {
            maxHeap.pop();
            maxHeap.push({ data, actual_dist });
            result.steps.push_back("更新最近邻: " + data->toString());
        }
        else {
            result.steps.push_back("排除 " + data->toString() + "（非最近邻）");
        }

        // 更新当前最近邻和最小距离
        if (actual_dist < result.distance) {
            result.distance = actual_dist;
            result.nearest = data;
        }
    }

    // 将最大堆中的元素取出并按升序排列
    while (!maxHeap.empty()) {
        result.knnResults.push_back(maxHeap.top());
        maxHeap.pop();
    }
    reverse(result.knnResults.begin(), result.knnResults.end());

    auto end = high_resolution_clock::now();
    result.timeMicrosec = duration_cast<microseconds>(end - start).count();

    return result;
}

void runKnnQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        cout << "错误：数据集为空。\n";
        return;
    }

    // 是否展示数据集
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

    // 输入 k 值
    int k;
    cout << "请输入要查找的最近邻个数 k：";
    cin >> k;

    if (cin.fail() || k <= 0 || k > static_cast<int>(dataset.size())) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "输入无效，请输入一个大于 0 并小于等于数据集大小的值。\n";
        return;
    }

    // 创建对应的距离函数
    shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 调用 knnQuery 进行搜索
    cout << "\n开始 kNN 搜索...\n";

    MetricSpaceSearch::SearchResult result = knnQuery(dataset, queryPtr, distanceFunc, k, pivot);

    // 使用统一打印函数输出结果
    printSearchResult(result);
}