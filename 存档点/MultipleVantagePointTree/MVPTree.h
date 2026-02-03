// MVPTree.h
#ifndef MVPTREE_H
#define MVPTREE_H

#include <vector>
#include <memory>
#include "MVPTNode.h"   
#include "../src/PivotSelector/PivotSelector.h"

class MetricDistance;

class MVPTree {
public:
    static std::unique_ptr<MVPTNode> bulkLoad(
        const std::vector<DataPtr>& data,
        int k, int f,
        int distanceType, int dataType,
        PivotSelector::SelectionMethod method);

    static void runMVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // 获取和重置距离计算次数
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    static long long distanceCalculations_;  // 新增成员变量

    static std::vector<std::vector<DataPtr>> splitByRadius(
        const std::vector<DataPtr>& data, const DataPtr& vp,
        int f, const MetricDistance& dist);

    static void computeBounds(
        const std::vector<DataPtr>& vpList,
        const std::vector<std::vector<DataPtr>>& partitions,
        const MetricDistance& dist,
        std::vector<std::vector<long double>>& lowerBounds,
        std::vector<std::vector<long double>>& upperBounds);
};

#endif // MVPTREE_H
