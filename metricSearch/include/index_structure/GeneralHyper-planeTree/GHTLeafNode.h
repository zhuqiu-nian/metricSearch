#pragma once

#include "GHTNode.h"
#include "../PivotTable/PivotTable.h"

class GHTLeafNode : public GHTNode {
public:
    explicit GHTLeafNode(const DataList& data, int distanceType, int dataType);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;

private:
    PivotTable pivotTable_;
};
