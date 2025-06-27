#include "../../include/metric_search/MetricRangeQuery.h"
#include "../../include/utils/Solution.h"
#include <chrono>
#include <cmath>
#include <iostream>


using namespace std;
using namespace std::chrono;

namespace MetricSpaceExtensions {

    MetricSpaceSearch::SearchResult rangeQuery(
        const vector<shared_ptr<MetricData>>& dataset,
        const shared_ptr<MetricData>& query,
        const shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const shared_ptr<MetricData>& pivot) {

        MetricSpaceSearch::SearchResult result;
        result.distanceFunc = distanceFunc;
        result.pivot = pivot ? pivot : (dataset.empty() ? nullptr : dataset[0]);
        result.calculations = 0;
        result.timeMicrosec = 0;

        if (dataset.empty() || !result.pivot) {
            auto end = high_resolution_clock::now();
            result.timeMicrosec = duration_cast<microseconds>(high_resolution_clock::now() - high_resolution_clock::now()).count();
            return result;
        }

        auto start = high_resolution_clock::now();

        // 获取支撑点到查询点的距离
        long double d_pq = distanceFunc->distance(*result.pivot, *query);
        result.calculations++;

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
                }
            }
            else {
                d_pd = distanceFunc->distance(*result.pivot, *data);
                result.calculations++;
            }

            // 三角不等式剪枝
            long double lower_bound = abs(d_pq - d_pd);
            long double upper_bound = d_pq + d_pd;

            if (lower_bound > radius) {
                result.steps.push_back("跳过 " + data->toString() + "（下界剪枝）");//实际距离＞下界＞半径，直接排除
                continue;
            }

            if (upper_bound <= radius) {
                result.rangeResults.emplace_back(data, -1.0L); // 半径＞上界＞实际距离，直接加入
                result.steps.push_back("加入 " + data->toString() + "（上界剪枝）");
                continue;
            }

            // 需要实际计算 query 到 data 的距离
            long double actual_dist = distanceFunc->distance(*query, *data);
            result.calculations++;
            if (actual_dist <= radius) {
                result.rangeResults.emplace_back(data, actual_dist);
                result.steps.push_back("加入 " + data->toString() + "（真实距离 ≤ 半径）");
            }
            else {
                result.steps.push_back("排除 " + data->toString() + "（真实距离 > 半径）");
            }
        }

        auto end = high_resolution_clock::now();
        result.timeMicrosec = duration_cast<microseconds>(end - start).count();

        return result;
    }

    void runRangeQuery(
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

        // 输入最大距离限制 radius
        long double radius;
        cout << "请输入最大搜索半径（radius）：";
        cin >> radius;

        if (cin.fail() || radius < 0.0L) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请输入一个非负数值。\n";
            return;
        }

        // 创建对应的距离函数
        shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

        // 调用 rangeQuery 进行搜索
        cout << "\n开始范围搜索...\n";

        MetricSpaceSearch::SearchResult result = MetricSpaceExtensions::rangeQuery(dataset, queryPtr, distanceFunc, radius, pivot);

        // 使用统一打印函数输出结果
        printSearchResult(result);
    }

} // namespace MetricSpaceExtensions
