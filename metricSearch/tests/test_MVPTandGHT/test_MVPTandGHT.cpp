// benchmarkFourTrees.cpp
#include "../include/utils/SystemLaunch.h"
#include "../include/index_structure/all_indexHead.h"
#include "../include/core/Data_subclass/VectorData.h"
#include "../include/utils/LoadData.h"
#include "../src/PivotSelector/PivotSelector.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

using namespace std;
using namespace chrono;

// 批量范围查询：仅支持 GHT / VPT / MVPT / AT
// 返回 {总距离计算次数, 总查询耗时（纳秒）}
pair<long long, long long> runBatchRangeSearch(
    const DataList& dataset,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method,
    long double radius,
    const string& treeType,
    int k_pivots = 2,   // MVPT 使用
    int f_splits = 3    // MVPT 使用
) {
    unique_ptr<GHTNode> ghtTree;
    unique_ptr<VPTNode> vptTree;
    unique_ptr<MVPTNode> mvptTree;
    unique_ptr<ApollonianNode> atTree;

    // 构建索引树（建树时间不计入查询耗时）
    if (treeType == "GHT") {
        ghtTree = GHTree::bulkLoad(dataset, distanceType, dataType, method);
    }
    else if (treeType == "VPT") {
        vptTree = VPTree::bulkLoad(dataset, distanceType, dataType, method);
    }
    else if (treeType == "MVPT") {
        mvptTree = MVPTree::bulkLoad(dataset, k_pivots, f_splits, distanceType, dataType, method);
    }
    else if (treeType == "AT") {
        atTree = ApollonianTree::bulkLoad(dataset, distanceType, dataType, method);
    }

    long long totalDistCount = 0;
    auto start = high_resolution_clock::now();

    for (size_t i = 0; i < dataset.size(); ++i) {
        long long localCounter = 0;
        const MetricData& query = *dataset[i];

        if (treeType == "GHT") {
            ghtTree->rangeSearch(query, radius, &localCounter);
        }
        else if (treeType == "VPT") {
            vptTree->rangeSearch(query, radius, &localCounter);
        }
        else if (treeType == "MVPT") {
            mvptTree->rangeSearch(query, radius, &localCounter);
        }
        else if (treeType == "AT") {
            atTree->rangeSearch(query, radius, &localCounter);
        }

        totalDistCount += localCounter;
    }

    auto end = high_resolution_clock::now();
    long long totalTimeNs = duration_cast<nanoseconds>(end - start).count();

    return make_pair(totalDistCount, totalTimeNs);
}

int run_test_VS() {
    vector<string> filenames = {
        "yeast.aa"
    };

    const int N = 100;
    const int distanceType = 1;  // Euclidean
    const int dataType = 3;      // Vector
    const long double radius = 2;
    const auto method = PivotSelector::MAX_VARIANCE;

    // 树参数
    const int k_pivots = 3;      // MVPT
    const int f_splits = 3;      // MVPT

    cout << fixed << setprecision(2);
    cout << "================================================================================\n";
    cout << "   范围查询性能对比：GHT vs VPT vs MVPT vs AT\n";
    cout << "================================================================================\n";
    cout << "参数设置：\n";
    cout << "  - 查询点数: " << N << "\n";
    cout << "  - 查询半径: " << radius << "\n";
    cout << "  - 距离类型: 欧氏距离 (Euclidean)\n";
    cout << "  - 枢轴选择策略: 最大方差\n";
    cout << "  - MVPT 参数: k_pivots=" << k_pivots << ", f_splits=" << f_splits << "\n";
    cout << "  - AT 参数: 自动根据局部数据分布确定划分比例（1/3 与 2/3 分位数）\n";
    cout << "--------------------------------------------------------------------------------\n";

    // 表头：4 列
    cout << left << setw(20) << "数据集"
        << right << setw(12) << "GHT(D)"
        << setw(12) << "VPT(D)"
        << setw(12) << "MVPT(D)"
        << setw(12) << "AT(D)" << "\n";
    cout << left << setw(20) << "(D=距离次数)"
        << right << setw(12) << "(T=μs)"
        << setw(12) << "(T=μs)"
        << setw(12) << "(T=μs)"
        << setw(12) << "(T=μs)" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    for (const auto& fname : filenames) {
        string full_path = "data/protein/" + fname;

        cout << "正在加载: " << fname << " ... ";
        fflush(stdout);

        auto vecData = loadProteinData(full_path, N);
        DataList dataset(vecData.begin(), vecData.end());

        if (dataset.empty()) {
            cerr << "\n错误：无法加载数据集 " << fname << endl;
            string name = (fname.length() > 18) ? fname.substr(0, 15) + "..." : fname;
            // 输出 4 列 N/A
            cout << left << setw(20) << name
                << right << setw(12) << "N/A"
                << setw(12) << "N/A"
                << setw(12) << "N/A"
                << setw(12) << "N/A" << '\n';
            cout << left << setw(20) << ""
                << right << setw(12) << "N/A"
                << setw(12) << "N/A"
                << setw(12) << "N/A"
                << setw(12) << "N/A" << '\n';
            cout << "--------------------------------------------------------------------------------\n";
            continue;
        }

        cout << "成功 (" << dataset.size() << " 个点)\n";

        // 重置全局距离计数器（如果支持）
        GHTree::resetDistanceCalculations();
        VPTree::resetDistanceCalculations();
        MVPTree::resetDistanceCalculations();
        // AT 使用局部计数，无需 reset

        // 执行四类树的批量查询
        auto ghtResult = runBatchRangeSearch(dataset, distanceType, dataType, method, radius, "GHT");
        auto vptResult = runBatchRangeSearch(dataset, distanceType, dataType, method, radius, "VPT");
        auto mvptResult = runBatchRangeSearch(dataset, distanceType, dataType, method, radius, "MVPT", k_pivots, f_splits);
        auto atResult = runBatchRangeSearch(dataset, distanceType, dataType, method, radius, "AT");

        size_t n = dataset.size();
        double ghtAvgDist = static_cast<double>(ghtResult.first) / n;
        double vptAvgDist = static_cast<double>(vptResult.first) / n;
        double mvptAvgDist = static_cast<double>(mvptResult.first) / n;
        double atAvgDist = static_cast<double>(atResult.first) / n;

        double ghtAvgTime = (ghtResult.second / 1000.0) / n;
        double vptAvgTime = (vptResult.second / 1000.0) / n;
        double mvptAvgTime = (mvptResult.second / 1000.0) / n;
        double atAvgTime = (atResult.second / 1000.0) / n;

        string displayName = (fname.length() > 18) ? fname.substr(0, 15) + "..." : fname;

        // 第一行：平均距离计算次数
        cout << left << setw(20) << displayName
            << right << setw(12) << ghtAvgDist
            << setw(12) << vptAvgDist
            << setw(12) << mvptAvgDist
            << setw(12) << atAvgDist << '\n';

        // 第二行：平均查询时间（微秒）
        cout << left << setw(20) << "(μs/query)"
            << right << setw(12) << ghtAvgTime
            << setw(12) << vptAvgTime
            << setw(12) << mvptAvgTime
            << setw(12) << atAvgTime << '\n';

        cout << "--------------------------------------------------------------------------------\n";
    }

    cout << "\n性能对比完成：D = 平均距离计算次数，T = 平均单次查询时间（微秒）\n";
    return 0;
}