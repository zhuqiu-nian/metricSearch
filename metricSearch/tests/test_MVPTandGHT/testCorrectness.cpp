// testCorrectness.cpp
#include "../include/utils/SystemLaunch.h"
#include "../include/index_structure/all_indexHead.h"
#include "../include/core/Data_subclass/VectorData.h"
#include "../include/utils/LoadData.h"
#include "../src/PivotSelector/PivotSelector.h"

// --------------------------------
// 新增：包含 ApollonianTree 头文件
#include "../include/index_structure/ApollonianTree/ApollonianTree.h"
// --------------------------------

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <string>
#include <set>
#include <algorithm>

using namespace std;

void runSingleRangeSearch(
    const DataList& dataset,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method,
    long double radius,
    const string& treeType,
    // 通用参数（MVPT/CLPT/GHT/CGHT 使用）
    int k_pivots = 2,
    int f_splits = 3,
    int cg_num_regions_per_dim = 3,
    // --- 新增：AT 特有参数 ---
    long double at_c1 = 0.5L,
    long double at_c2 = 2.0L
    // ------------------------
) {
    unique_ptr<GHTNode> ghtTree;
    unique_ptr<MVPTNode> mvptTree;
    unique_ptr<CGHTNode> cghtTree;
    unique_ptr<CLPTNode> clptTree;
    // --- 新增：AT 根节点 ---
    unique_ptr<ApollonianNode> atTree;
    // -----------------------

    // 构建索引树
    if (treeType == "GHT") {
        ghtTree = GHTree::bulkLoad(dataset, distanceType, dataType, method);
    }
    else if (treeType == "MVPT") {
        mvptTree = MVPTree::bulkLoad(dataset, k_pivots, f_splits, distanceType, dataType, method);
    }
    else if (treeType == "CGHT") {
        int actual_f_splits = max(1, cg_num_regions_per_dim - 1);
        cghtTree = CGHTree::bulkLoad(dataset, distanceType, dataType, method, actual_f_splits);
    }
    else if (treeType == "CLPT") {
        clptTree = CLPTree::bulkLoad(dataset, k_pivots, f_splits, distanceType, dataType, method);
    }
    // --- 新增：构建 AT ---
    else if (treeType == "AT") {
        atTree = ApollonianTree::bulkLoad(dataset, distanceType, dataType, method);
    }
    // ---------------------

    if (dataset.empty()) {
        cout << "数据集为空，无法进行查询。\n";
        return;
    }

    const MetricData& query = *dataset[0];
    cout << "\n--- " << treeType << " ---" << endl;
    cout << "查询对象: " << query.toString() << endl;

    // 重置距离计数器（如果使用全局静态变量）
    // 注意：AT 目前使用局部计数，但为统一风格，我们仍保留 reset（可选）
    if (treeType == "GHT") {
        GHTree::resetDistanceCalculations();
    }
    else if (treeType == "MVPT") {
        MVPTree::resetDistanceCalculations();
    }
    else if (treeType == "CGHT") {
        CGHTree::resetDistanceCalculations();
    }
    else if (treeType == "CLPT") {
        CLPTree::resetDistanceCalculations();
    }
    // AT 不需要 reset（因为我们传局部指针），但可留空或忽略

    long long distCounter = 0;

    if (treeType == "GHT") {
        ghtTree->rangeSearch(query, radius, &distCounter);
    }
    else if (treeType == "MVPT") {
        mvptTree->rangeSearch(query, radius, &distCounter);
    }
    else if (treeType == "CGHT") {
        cghtTree->rangeSearch(query, radius, &distCounter);
    }
    else if (treeType == "CLPT") {
        clptTree->rangeSearch(query, radius, &distCounter);
    }
    // --- 新增：执行 AT 查询 ---
    else if (treeType == "AT") {
        atTree->rangeSearch(query, radius, &distCounter);
    }
    // -------------------------

    // 暴力搜索作为 ground truth
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    vector<const MetricData*> bruteResults;
    for (const auto& data : dataset) {
        if (data.get() != &query) {
            long double d = dist->distance(query, *data);
            if (d <= radius) {
                bruteResults.push_back(data.get());
            }
        }
    }

    cout << "找到匹配项数量（不包括查询对象自身）: " << bruteResults.size() << endl;
    cout << "距离计算次数: " << distCounter << endl;

    if (!bruteResults.empty()) {
        cout << "以下是匹配项（最多显示10个）：" << endl;
        for (size_t i = 0; i < min(static_cast<size_t>(10), bruteResults.size()); ++i) {
            cout << "  - 匹配项 #" << i + 1 << ": " << bruteResults[i]->toString() << endl;
        }
        if (bruteResults.size() > 10) {
            cout << "  ... 还有 " << (bruteResults.size() - 10) << " 个结果未显示" << endl;
        }
    }
    else {
        cout << "未找到任何匹配项。" << endl;
    }
}

int run_test_correctness() {
    string filename = "clusteredvector-2d-100k-100c.txt";
    string full_path = "data/vector/" + filename;

    const int N = 1000;
    const int distanceType = 1;  // Euclidean
    const int dataType = 1;      // Vector
    const long double radius = 0.0002L;
    const auto method = PivotSelector::MAX_VARIANCE;

    // 通用参数
    const int k_pivots = 2;
    const int f_splits = 3;
    const int cg_num_regions_per_dim = 3;

    // --- 新增：AT 参数 ---
    const long double at_c1 = 0.5L;
    const long double at_c2 = 2.0L;
    // ---------------------

    cout << fixed << setprecision(2);
    cout << "==================================================================\n";
    cout << "    算法正确性验证：GHT vs MVPT vs CGHT vs CLPT vs AT\n";
    cout << "==================================================================\n";
    cout << "参数设置：\n";
    cout << "  - 数据集: " << filename << " (前" << N << "个点)\n";
    cout << "  - 查询半径: " << radius << "\n";
    cout << "  - 距离类型: 欧氏距离 (Euclidean)\n";
    cout << "  - 枢轴选择策略: 最大方差\n";
    cout << "  - MVPT/CLPT 参数: k_pivots=" << k_pivots << ", f_splits=" << f_splits << "\n";
    cout << "  - CGHT 参数: num_regions_per_dim=" << cg_num_regions_per_dim << "\n";
    cout << "==================================================================\n";

    cout << "正在加载: " << filename << " ... ";
    fflush(stdout);

    auto vecData = loadVectorData(full_path, N);
    DataList dataset(vecData.begin(), vecData.end());

    if (dataset.empty()) {
        cerr << "\n错误：无法加载数据集 " << filename << endl;
        return -1;
    }

    cout << "成功 (" << dataset.size() << " 个点)\n";

    // 执行各算法的单次查询
    runSingleRangeSearch(dataset, distanceType, dataType, method, radius, "GHT");
    runSingleRangeSearch(dataset, distanceType, dataType, method, radius, "MVPT", k_pivots, f_splits);
    runSingleRangeSearch(dataset, distanceType, dataType, method, radius, "CGHT", 0, 0, cg_num_regions_per_dim);
    runSingleRangeSearch(dataset, distanceType, dataType, method, radius, "CLPT", k_pivots, f_splits);
    // --- 新增：测试 AT ---
    runSingleRangeSearch(dataset, distanceType, dataType, method, radius, "AT", 0, 0, 0, at_c1, at_c2);
    // ---------------------

    cout << "\n==================================================================\n";
    cout << "算法正确性验证完成\n";
    cout << "所有算法应返回相同的结果集（匹配项数量一致）\n";
    cout << "==================================================================\n";

    return 0;
}