#include "../../../include/index_structure/GeneralHyper-planeTree/GHTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"

GHTLeafNode::GHTLeafNode(const DataList& data, int distanceType, int dataType)
    : pivotTable_(data, 4, distanceType, dataType) {}  // 默认选4个支撑点

std::vector<DataPtr> GHTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    return pivotTable_.search(q, r, distanceCount);  // 已经会过滤掉查询对象本身
}