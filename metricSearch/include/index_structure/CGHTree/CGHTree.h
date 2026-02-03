// CGHTree.h
#ifndef CGHTREE_H
#define CGHTREE_H

#include <vector>
#include <memory>
#include "../src/PivotSelector/PivotSelector.h"
#include "CGHTNode.h"

class CGHTree {
public:
    static std::unique_ptr<CGHTNode> bulkLoad(const std::vector<std::shared_ptr<MetricData>>& data,
        int distanceType,
        int dataType,
        PivotSelector::SelectionMethod method,
        int f = 2);  // 新增 f 参数，默认为 2

    static void runCGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // 获取和重置距离计算次数
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    static long long distanceCalculations_;

    // 三维聚类分区：每个维度划分 f 次，总共 f^3 个子区域
    static std::vector<std::vector<std::tuple<DataPtr, long double, long double, long double>>>
        clusterPartition(std::vector<std::tuple<DataPtr, long double, long double, long double>>& distances,
            int f);

};

#endif // CGHTREE_H