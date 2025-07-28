// MVPTLeafNode.h
#ifndef MVPTLEAFNODE_H
#define MVPTLEAFNODE_H

#include "MVPTNode.h"
#include "../PivotTable/PivotTable.h"

class MVPTLeafNode : public MVPTNode {
public:
    explicit MVPTLeafNode(const DataList& data, int distanceType, int dataType, std::vector<int> selectedPivots);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    std::unique_ptr<PivotTable> pivotTable_;  // 改为指针
    DataList dataList_;
    bool isEmpty_;                 // 新增：标记是否为空节点
    int distanceType_;
    int dataType_;
};

#endif // MVPTLEAFNODE_H
