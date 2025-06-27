// VPTLeafNode.h
#ifndef VPTLEAFNODE_H
#define VPTLEAFNODE_H

#include "VPTNode.h"
#include "../PivotTable/PivotTable.h"

class VPTLeafNode : public VPTNode {
public:
    explicit VPTLeafNode(const DataList& data, int distanceType, int dataType);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) ;
    DataList getAll() const override;

private:
    PivotTable pivotTable_;
    DataList dataList_;  // 保存原始数据，用于 getAll()
};

#endif // VPTLEAFNODE_H#pragma once
