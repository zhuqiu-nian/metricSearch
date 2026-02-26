#pragma once

#include "GHTNode.h"
#include "../../interfaces/MetricDistance.h"  // 包含 MetricDistance 及其子类定义
#include "../../interfaces/MetricData.h"
#include "../src/PivotSelector/PivotSelector.h"
#include <memory>

class GHTree {
public:
    static std::unique_ptr<GHTNode> bulkLoad(const DataList& data,
        int distanceType,
        int dataType,
        PivotSelector::SelectionMethod method);

    static void runGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // 获取和重置距离计算次数
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    static long long distanceCalculations_;  // 新增成员变量

    static std::pair<DataPtr, DataPtr> selectPivots(const DataList& data);
};