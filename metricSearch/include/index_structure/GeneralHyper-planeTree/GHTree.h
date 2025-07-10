#pragma once

#include "GHTNode.h"
#include "../../interfaces/MetricDistance.h"  // ���� MetricDistance �������ඨ��
#include "../../interfaces/MetricData.h"
#include <memory>

class GHTree {
public:
    static std::unique_ptr<GHTNode> bulkLoad(const DataList& data,
        int distanceType,
        int dataType);

    static void runGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // ��ȡ�����þ���������
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    static long long distanceCalculations_;  // ������Ա����

    static std::pair<DataPtr, DataPtr> selectPivots(const DataList& data);
};