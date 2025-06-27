#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/Solution.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include "../../include/utils/LoadData.h"
#include "../../include/metric_search/all_search.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <stdexcept>

using namespace std;
using namespace std::chrono;
using DataList = std::vector<std::shared_ptr<MetricData>>;




// 实验运行函数（轮询模式，展示性能），这个函数改到一半，我发现送入的dataset只使用了一种距离函数，所以并不能进行多距离函数轮询，所以我重写了一个可以对多距离函数轮询的函数
void runExperiments(
    const vector<shared_ptr<MetricData>>& dataset,
    int data_num,
    int enableAllDistances,  // 0=所有；1/2/3=指定单个
    int data_var)            // 1=向量；2=字符串;3=蛋白质
{
    cout << "[信息] 开始运行多距离函数实验..." << endl;

    if (dataset.empty()) {
        cerr << "[错误] 数据集为空。" << endl;
        return;
    }

    vector<shared_ptr<MetricDistance>> distanceFuncs;

    if (enableAllDistances == 0) {
        // 使用该组的所有距离函数
        distanceFuncs = MetricSpaceSearch::getAllDistanceFunctions(data_var);
    }
    else {
        // 使用指定的单个距离函数
        auto func = MetricSpaceSearch::createDistanceFunction(enableAllDistances, data_var);
        distanceFuncs = { func };
    }

    for (int i = 0; i < data_num && i < dataset.size(); ++i) {
        auto query = dataset[i];
        cout << "\n\n===== 实验: 查询对象为 " << query->toString() << " =====" << endl;

        vector<vector<MetricSpaceSearch::SearchResult>> allRoundResults;

        // 测试每个支撑点
        for (int j = 0; j < data_num && j < dataset.size(); ++j) {
            if (i == j) continue;

            auto pivot = dataset[j];
            cout << "\n--- 使用支撑点: " << pivot->toString() << " ---" << endl;

            vector<MetricSpaceSearch::SearchResult> roundResults;

            // 对每种距离函数执行一次查询
            for (const auto& distFunc : distanceFuncs) {
                cout << "[信息] 使用距离函数: " << distFunc->getName() << endl;

                auto start = high_resolution_clock::now();
                auto result = MetricSpaceNNQuery::nearestNeighbor(dataset, query, distFunc, pivot);
                auto end = high_resolution_clock::now();

                result.timeMicrosec = duration_cast<microseconds>(end - start).count();
                result.distanceFunc = distFunc;
                result.pivot = pivot;

                // 输出结果
                cout << "最近邻: " << result.nearest->toString() << endl;
                cout << "距离: " << result.distance << endl;
                cout << "计算次数: " << result.calculations << endl;
                cout << "耗时: " << result.timeMicrosec << " μs" << endl;

                cout << "[搜索过程]" << endl;
                for (const auto& step : result.steps) {
                    cout << "  " << step << endl;
                }

                roundResults.push_back(result);
            }

            allRoundResults.push_back(roundResults);
        }

        // 综合分析
        MetricSpaceSearch::analyzePerformance(dataset, allRoundResults, query);
    }
}

// 实验运行函数（查询模式）
void runNearestNeighbor(
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

    // 创建对应的距离函数
    shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // 调用 nearestNeighbor 进行搜索
    cout << "\n开始最近邻搜索...\n";

    MetricSpaceSearch::SearchResult result = MetricSpaceNNQuery::nearestNeighbor(dataset, queryPtr, distanceFunc, pivot);

    // 输出结果
    cout << "\n--- 搜索结果 ---\n";
    cout << "查询点: " << queryPtr->toString() << endl;
    cout << "支撑点: " << result.pivot->toString() << endl;
    if (result.nearest) {
        cout << "最近邻点: " << result.nearest->toString() << endl;
        cout << "与查询点的最小距离为: " << result.distance << endl;
        cout << "实际计算次数: " << result.calculations << endl;
    }
    else {
        cout << "未找到最近邻点。\n";
    }

    cout << "\n--- 搜索过程记录 ---\n";
    for (const auto& step : result.steps) {
        cout << step << endl;
    }
    cout << "------------------------\n";
}




// 自定义输入查询点（返回 shared_ptr<MetricData>）
shared_ptr<MetricData> inputCustomQuery(int dataType) {
    if (dataType == 1) { // 向量数据
        int id;
        cout << "请输入自定义查询点的 ID: ";
        cin >> id;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("ID 输入无效");
        }

        int dim;
        cout << "请输入向量维度：";
        cin >> dim;

        if (cin.fail() || dim <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("维度输入无效或小于等于0");
        }

        vector<long double> vec(dim);
        cout << "请输入维度值（空格分隔）：";
        for (int i = 0; i < dim; ++i) {
            cin >> vec[i];
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw runtime_error("数值输入无效");
            }
        }

        return make_shared<VectorData>(vec, id);
    }
    else if (dataType == 2) { // 字符串数据
        int id;
        cout << "请输入自定义字符串对象的 ID: ";
        cin >> id;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("ID 输入无效");
        }

        string str;
        cout << "请输入字符串内容：";
        cin >> str;

        return make_shared<StringData>(str, id);
    }
    else {
        throw runtime_error("不支持的数据类型");
    }
}

//统一输出函数逻辑
void printSearchResult(const MetricSpaceSearch::SearchResult& result) {
    cout << "\n--- 搜索结果 ---\n";
    cout << "支撑点: " << result.pivot->toString() << endl;
    cout << "实际计算次数: " << result.calculations << endl;
    cout << "查询耗时: " << result.timeMicrosec << " 微秒" << endl;

    // 范围查询结果
    if (!result.rangeResults.empty()) {
        cout << "范围查询匹配数量: " << result.rangeResults.size() << endl;
        cout << "\n--- 匹配的数据点（按距离升序）---\n";
        for (const auto& entry : result.rangeResults) {
            const shared_ptr<MetricData>& point = entry.first;
            long double distance = entry.second;
            cout << point->toString() << " | 距离: " << distance << endl;
        }
    }
    else {
        cout << "范围查询匹配数量: 无\n";
    }

    // kNN 查询结果
    if (!result.knnResults.empty()) {
        cout << "kNN 查询结果数量: " << result.knnResults.size() << " / k=" << result.k << endl;
        cout << "\n--- 最近邻点列表（按距离升序）---\n";
        for (const auto& entry : result.knnResults) {
            const shared_ptr<MetricData>& neighbor = entry.first;
            long double distance = entry.second;
            cout << neighbor->toString() << " | 距离: " << distance << endl;
        }
    }
    else {
        cout << "kNN 匹配数量: 无\n";
    }

    // 距离受限的 kNN 查询结果
    if (!result.boundedKnnResults.empty()) {
        cout << "距离受限 kNN 结果数量: " << result.boundedKnnResults.size()
            << " / boundedK=" << result.boundedK
            << " / radius=" << result.radius << endl;
        cout << "\n--- 最近邻点列表（按距离升序）---\n";
        for (const auto& entry : result.boundedKnnResults) {
            const shared_ptr<MetricData>& neighbor = entry.first;
            long double distance = entry.second;
            cout << neighbor->toString() << " | 距离: " << distance << endl;
        }
    }
    else {
        cout << "距离受限 kNN 匹配数量: 无\n";
    }

    // 查询过程日志
    cout << "\n--- 搜索过程记录 ---\n";
    for (const auto& step : result.steps) {
        cout << "- " << step << endl;
    }
    cout << "------------------------\n";
}






