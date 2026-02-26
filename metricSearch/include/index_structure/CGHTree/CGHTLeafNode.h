// CGHTLeafNode.h
#ifndef CGHTLEAFNODE_H
#define CGHTLEAFNODE_H

#include "CGHTNode.h"
#include "../PivotTable/PivotTable.h"
#include "../src/PivotSelector/PivotSelector.h"

class CGHTLeafNode : public CGHTNode {
public:
    CGHTLeafNode(const DataList& data, int distanceType, int dataType,
        PivotSelector::SelectionMethod method, int pivotCountForLeaf = 1); // ← 添加参数

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount);
    DataList getAll() const;

private:
    DataList dataList_;
    int distanceType_;
    int dataType_;
    bool isEmpty_;

    // 移除原有的 pivot1_, pivot2_
    // DataPtr pivot1_, pivot2_; 

    // 添加 PivotTable 成员
    std::unique_ptr<PivotTable> pivotTable_;
};

#endif // CGHTLEAFNODE_H