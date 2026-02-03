#include "../../../include/index_structure/PivotTable/PivotTable.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"
#include <stdexcept>
#include <limits>
#include <iostream>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace std::chrono;

long long PivotTable::distanceCalculations_ = 0;

// 批建构造函数（传入支撑点序列）
PivotTable::PivotTable(const vector<shared_ptr<MetricData>>& allData,
    const vector<int>& pivotIndices,
    int distanceType,
    int data_var)
{
    // 如果数据集为空
    if (allData.empty()) {
        if (pivotIndices.empty()) {
            buildEmpty(distanceType, data_var);
            return;
        }
        else {
            throw invalid_argument("数据集为空但请求构建支撑点");
        }
    }



    auto start = high_resolution_clock::now();

    // 过滤合法的支撑点索引
    vector<int> validIndices;
    for (int idx : pivotIndices) {
        if (idx >= 0 && idx < static_cast<int>(allData.size())) {
            validIndices.push_back(idx);
        }
        // 可选：输出警告日志
        // else { cout << "[警告] 支撑点索引 " << idx << " 越界，已跳过" << endl; }
    }

    // 如果没有合法支撑点，退化为空表
    if (validIndices.empty()) {
        buildEmpty(distanceType, data_var);
        return;
    }

    // 提取支撑点
    for (int idx : validIndices) {
        pivots_.push_back(allData[idx]);
    }

    // 将非支撑点加入 data_
    for (size_t i = 0; i < allData.size(); ++i) {
        bool isPivot = false;
        for (int pivotIdx : validIndices) {
            if (static_cast<size_t>(pivotIdx) == i) {
                isPivot = true;
                break;
            }
        }
        if (!isPivot) {
            data_.push_back(allData[i]);
        }
    }

    // 构建距离表（同时创建距离函数）
    buildDistanceTable(distanceType, data_var);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);



    selectedPivotIndices_ = validIndices;
}

// 构建距离表：支撑点到每个非支撑点的距离
void PivotTable::buildDistanceTable(int distanceType, int data_var) {

    distance_ = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);  // 创建距离函数

    if (pivots_.empty() || data_.empty()) {
        distanceTable_.clear();
        return;
    }

    distanceTable_.resize(pivots_.size());
    for (size_t p = 0; p < pivots_.size(); ++p) {
        distanceTable_[p].resize(data_.size());
        for (size_t d = 0; d < data_.size(); ++d) {
            distanceTable_[p][d] = distance_->distance(*pivots_[p], *data_[d]);
        }
    }
}

// 批建构造函数（传入支撑点个数）
PivotTable::PivotTable(const vector<shared_ptr<MetricData>>& allData,
    int k,
    int distanceType,
    int data_var)
{
    // 处理空数据
    if (allData.empty()) {
        if (k == 0) {
            buildEmpty(distanceType, data_var);
            return;
        }
        if (k > 0) {
            std::cerr << "[PivotTable] 警告: 数据集为空但请求 k=" << k << "，降级为空表" << std::endl;
        }
        buildEmpty(distanceType, data_var);
        return;
    }

    // 自动调整 k
    int actual_k = std::min(k, static_cast<int>(allData.size()));
    if (actual_k <= 0) {
        buildEmpty(distanceType, data_var);
        return;
    }

    cout << "[预处理] 开始计算支撑点距离..." << endl;
    cout << "[预处理] 数据集大小: " << allData.size() << endl;

    auto start = high_resolution_clock::now();

    // 随机选取支撑点索引
    vector<int> pivotIndices = selectRandomPivots(allData.size(), actual_k);
    selectedPivotIndices_ = pivotIndices;

    // 提取支撑点
    for (int idx : pivotIndices) {
        pivots_.push_back(allData[idx]);
    }

    // 提取非支撑点
    for (size_t i = 0; i < allData.size(); ++i) {
        bool isPivot = false;
        for (int pivotIdx : pivotIndices) {
            if (static_cast<size_t>(pivotIdx) == i) {
                isPivot = true;
                break;
            }
        }
        if (!isPivot) {
            data_.push_back(allData[i]);
        }
    }

    // 构建距离表
    buildDistanceTable(distanceType, data_var);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "[预处理] 完成！" << endl;
    cout << "[预处理] 缓存条目数: " << pivots_.size() * data_.size() << endl;
    cout << "[预处理] 耗时: " << duration.count() << " ms" << endl;
}


// 基于三角不等式的搜索
vector<shared_ptr<MetricData>> PivotTable::search(
    const MetricData& query,
    long double threshold,
    long long* distanceCount) const
{
    vector<shared_ptr<MetricData>> result;

    //保护：如果无数据，直接返回
    if (data_.empty() && pivots_.empty()) {
        return result;
    }

    // 第一步：计算所有支撑点到查询对象的距离
    vector<long double> qDists(pivots_.size());
    for (size_t p = 0; p < pivots_.size(); ++p) {
        qDists[p] = distance_->distance(*pivots_[p], query);
        if (distanceCount) (*distanceCount)++;
        else PivotTable::distanceCalculations_++;
    }

    // 第二步：将满足条件的 pivot 直接加入结果集
    for (size_t p = 0; p < pivots_.size(); ++p) {
        if (qDists[p] <= threshold) {
            result.push_back(pivots_[p]);
        }
    }

    // 第三步：处理每个非支撑点数据
    for (size_t d = 0; d < data_.size(); ++d) {
        bool done = false;

        for (size_t p = 0; p < pivots_.size(); ++p) {
            long double pivotToQuery = qDists[p];
            long double pivotToData = distanceTable_[p][d];

            // 包含规则：pivotToQuery + pivotToData <= threshold
            if (pivotToQuery + pivotToData <= threshold) {
                result.push_back(data_[d]);
                done = true;
                break;
            }

            // 排除规则：abs(pivotToQuery - pivotToData) > threshold
            if (abs(pivotToQuery - pivotToData) > threshold) {
                done = true;
                break;
            }
        }

        // 若无法判断，则计算实际距离
        if (!done) {
            long double actualDist = distance_->distance(*data_[d], query);
            if (distanceCount) (*distanceCount)++;
            else PivotTable::distanceCalculations_++;
            if (actualDist <= threshold) {
                result.push_back(data_[d]);
            }
        }
    }

    return result;
}

vector<int> PivotTable::selectRandomPivots(int totalSize, int k) const {
    vector<int> indices(totalSize);
    iota(indices.begin(), indices.end(), 0);  // 0 ~ totalSize - 1

    // 使用随机洗牌算法（Fisher-Yates Shuffle）
    random_device rd;
    mt19937 g(rd());

    shuffle(indices.begin(), indices.end(), g);

    // 取前 k 个
    return vector<int>(indices.begin(), indices.begin() + k);
}

//运行Pivot Table

void PivotTable::interactiveRangeSearch(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "数据集为空，无法执行查询。" << std::endl;
        return;
    }

    // --- 1. 输入支撑点个数 ---
    int pivotCount;
    std::cout << "请输入支撑点个数: ";
    std::cin >> pivotCount;

    if (pivotCount <= 0 || pivotCount >= static_cast<int>(dataset.size())) {
        std::cerr << "支撑点个数必须大于0且小于数据总数。" << std::endl;
        return;
    }

    // --- 2. 选择支撑点选择算法 ---
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    // --- 3. 创建距离函数 ---
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);  // 创建距离函数

    // --- 4. 调用 PivotSelector 生成支撑点索引 ---
    double alpha = 0.35; // 稀疏空间法参数
    std::vector<int> selectedPivots = PivotSelector::selectPivots(
        dataset,
        pivotCount,
        dist,
        method,
        alpha
    );

    if (selectedPivots.empty()) {
        std::cerr << "未能生成有效的支撑点列表。" << std::endl;
        return;
    }

    // --- 5. 输入查询参数 ---
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

    // --- 6. 构建 PivotTable ---
    PivotTable pt(dataset, selectedPivots, distanceType, dataType);//!!!!!!!!PivotTable我实现了两种构建算法，即传入支撑点集和支撑点个数的，这里使用的是第一种，所以没有问题

    // 重置计数器
    PivotTable::resetDistanceCalculations();

    // 获取查询对象
    const MetricData& query = *queryPtr;

    // 执行范围查询
    auto results = pt.search(query, threshold);// 不传外部计数器时使用类内变量

    //过滤掉查询对象本身
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {  // 比较指针地址，判断是否是同一个对象
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
    std::cout << "\n本次查询共调用距离函数: " << PivotTable::getDistanceCalculations() << " 次（来自 PivotTable 内部统计）" << std::endl;
}

void PivotTable::buildEmpty(int distanceType, int data_var) {
    // 清理已有数据（防御性编程）
    data_.clear();
    pivots_.clear();
    distanceTable_.clear();
    selectedPivotIndices_.clear();

    // 创建距离函数
    distance_ = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);

    // distanceCalculations_ 是 static，无需在此初始化
}