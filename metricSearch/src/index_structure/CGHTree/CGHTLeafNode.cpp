// CGHTLeafNode.cpp
#include "../../../include/index_structure/CGHTree/CGHTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../PivotSelector/PivotSelector.h"
#include <random>

CGHTLeafNode::CGHTLeafNode(
    const DataList& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method,
    int pivotCountForLeaf)  // ← 添加此参数，默认为1，与MVPT一致
    : dataList_(data), distanceType_(distanceType), dataType_(dataType), isEmpty_(true), pivotTable_(nullptr)
{
    if (data.empty()) {
        return; // 空节点
    }

    try {
        // 确保 pivotCountForLeaf 不超过数据量
        int actualK = std::min(pivotCountForLeaf, static_cast<int>(data.size()));

        // 创建距离函数（用于 PivotSelector）
        auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

        // 在本地 data 上选择 actualK 个支撑点
        std::vector<int> selectedPivots;
        if (actualK == 1 && method == PivotSelector::RANDOM) {
            // 小优化：单点随机可不用完整 PivotSelector
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, static_cast<int>(data.size()) - 1);
            selectedPivots = { dis(gen) };
        }
        else {
            // 使用统一 PivotSelector 接口（支持 MaxVar / FFT 等）
            selectedPivots = PivotSelector::selectPivots(data, actualK, dist, method, 0.35);
        }

        // 构建 PivotTable
        pivotTable_ = std::make_unique<PivotTable>(data, selectedPivots, distanceType, dataType);
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        std::cerr << "[CGHTLeafNode] 构造 PivotTable 失败: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> CGHTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    // 如果没有 PivotTable 或者节点为空，则返回空结果
    if (!pivotTable_) {
        return {};
    }

    // 调用 PivotTable 的搜索方法
    return pivotTable_->search(q, r, distanceCount);
}

DataList CGHTLeafNode::getAll() const {
    return dataList_;
}