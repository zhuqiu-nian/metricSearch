// VPTLeafNode.h
#ifndef VPTLEAFNODE_H
#define VPTLEAFNODE_H

#include "VPTNode.h"
#include "../PivotTable/PivotTable.h"

class VPTLeafNode : public VPTNode {
public:
    explicit VPTLeafNode(const DataList& data, int distanceType, int dataType, std::vector<int> selectedPivots);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) ;
    DataList getAll() const override;

private:
    std::unique_ptr<PivotTable> pivotTable_;  // 改为指针
    DataList dataList_;
    bool isEmpty_;                 // 新增：标记是否为空节点
    int distanceType_;
    int dataType_;
};

#endif // VPTLEAFNODE_H#pragma once
