// CLPTLeafNode.cpp
#include "../../../include/index_structure/CLPTree/CLPTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include "../../../src/PivotSelector/PivotSelector.h"
#include <random>
#include <iostream>

CLPTLeafNode::CLPTLeafNode(
    const DataList& data,
    int distanceType,
    int dataType,
    PivotSelector::SelectionMethod method,
    int pivotCountForLeaf)
    : dataList_(data), isEmpty_(true)
{
    if (data.empty()) {
        return;
    }

    try {
        int actualK = std::min(pivotCountForLeaf, static_cast<int>(data.size()));
        auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

        std::vector<int> selectedPivots;
        if (actualK == 1 && method == PivotSelector::RANDOM) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, static_cast<int>(data.size()) - 1);
            selectedPivots = { dis(gen) };
        }
        else {
            selectedPivots = PivotSelector::selectPivots(data, actualK, dist, method, 0.35);
        }

        pivotTable_ = std::make_unique<PivotTable>(data, selectedPivots, distanceType, dataType);
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        std::cerr << "[CLPTLeafNode] 构造 PivotTable 失败: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> CLPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    if (!pivotTable_) {
        return {};
    }
    return pivotTable_->search(q, r, distanceCount); // ← 委托给 PivotTable
}

DataList CLPTLeafNode::getAll() const {
    return dataList_;
}