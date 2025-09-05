#pragma once

#include "GHTNode.h"
#include "../PivotTable/PivotTable.h"

class GHTLeafNode : public GHTNode {
public:
    explicit GHTLeafNode(const DataList& data, int distanceType, int dataType, std::vector<int> selectedPivots);
    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;

private:
    std::unique_ptr<PivotTable> pivotTable_;  // ��Ϊָ��
    DataList dataList_;
    bool isEmpty_;                 // ����������Ƿ�Ϊ�սڵ�
    int distanceType_;
    int dataType_;
};
