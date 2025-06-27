// MVPTLeafNode.cpp

#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"

MVPTLeafNode::MVPTLeafNode(const DataList& data, int distanceType, int dataType)
    : pivotTable_(data,1, distanceType, dataType), dataList_(data) {}

std::vector<DataPtr> MVPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    return pivotTable_.search(q, r, distanceCount);
}

DataList MVPTLeafNode::getAll() const {
    return dataList_;
}