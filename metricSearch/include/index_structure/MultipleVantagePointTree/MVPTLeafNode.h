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
    std::unique_ptr<PivotTable> pivotTable_;  // ��Ϊָ��
    DataList dataList_;
    bool isEmpty_;                 // ����������Ƿ�Ϊ�սڵ�
    int distanceType_;
    int dataType_;
};

#endif // MVPTLEAFNODE_H
