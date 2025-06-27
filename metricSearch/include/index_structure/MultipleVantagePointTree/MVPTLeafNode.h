// MVPTLeafNode.h
#ifndef MVPTLEAFNODE_H
#define MVPTLEAFNODE_H

#include "MVPTNode.h"
#include "../PivotTable/PivotTable.h"

class MVPTLeafNode : public MVPTNode {
public:
    explicit MVPTLeafNode(const DataList& data, int distanceType, int dataType);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    PivotTable pivotTable_;
    DataList dataList_;
};

#endif // MVPTLEAFNODE_H
