// VPTLeafNode.cpp
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../PivotSelector/PivotSelector.h"
#include <stdexcept>

VPTLeafNode::VPTLeafNode(const DataList& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method)
    : dataList_(data), distanceType_(distanceType), dataType_(dataType), isEmpty_(true)
{
    if (data.empty()) {
        return; // 空节点
    }

    try {
        auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
        // 从当前叶子数据中动态选择 1 个 pivot
        std::vector<int> pivots = PivotSelector::selectPivots(data, 1, dist, method, 0.35);
        pivotTable_ = std::make_unique<PivotTable>(data, pivots, distanceType, dataType);
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        std::cerr << "[VPTLeafNode] 构造 PivotTable 失败: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> VPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    if (!pivotTable_) {
        return {};
    }
    return pivotTable_->search(q, r, distanceCount);
}

DataList VPTLeafNode::getAll() const {
    return dataList_;
}