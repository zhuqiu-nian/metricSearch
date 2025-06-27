// VPTLeafNode.cpp
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"

VPTLeafNode::VPTLeafNode(const DataList& data, int distanceType, int dataType)
    : pivotTable_(data, 2, distanceType, dataType), dataList_(data) {}

std::vector<DataPtr> VPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount)  {
    return pivotTable_.search(q, r, distanceCount);
}

DataList VPTLeafNode::getAll() const {
    return dataList_;
}