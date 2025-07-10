// VPTree.h
#ifndef VPTREE_H
#define VPTREE_H

#include <vector>
#include <memory>
#include "VPTNode.h"

class VPTree {
public:
    static std::unique_ptr<VPTNode> bulkLoad(const std::vector<std::shared_ptr<MetricData>>& data,
        int distanceType,
        int dataType);

    static void runVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // ��ȡ�����þ���������
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    static long long distanceCalculations_;  // ������Ա����

    static std::pair<DataPtr, long double> selectVPAndRadius(
        const std::vector<DataPtr>& data,
        int distanceType,
        int dataType);

    static long double getMedianRadius(
        const std::vector<DataPtr>& data,
        const MetricData& vp,
        const MetricDistance& dist);
};

#endif // VPTREE_H