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
<<<<<<< HEAD
    std::unique_ptr<PivotTable> pivotTable_;  // ��Ϊָ��
    DataList dataList_;
    bool isEmpty_;                 // ����������Ƿ�Ϊ�սڵ�
    int distanceType_;
    int dataType_;
=======
    PivotTable pivotTable_;
    DataList dataList_;  // ����ԭʼ���ݣ����� getAll()
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
};

#endif // VPTLEAFNODE_H#pragma once
