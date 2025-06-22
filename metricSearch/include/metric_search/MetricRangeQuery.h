#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "../interfaces/MetricData.h"
#include "../interfaces/MetricDistance.h"
#include "../utils/MetricSpaceSearch.h"

namespace MetricSpaceExtensions {

    // 范围查询结果结构体
    struct RangeResult {
        std::vector<std::shared_ptr<MetricData>> results; // 符合条件的数据点
        int calculations = 0;                             // 实际计算距离次数
        long timeMicrosec = 0;                            // 查询耗时（微秒）
        std::string steps;                                // 查询过程日志
    };

    // 基于三角不等式的度量空间范围查询
    RangeResult rangeQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const std::shared_ptr<MetricData>& pivot = nullptr);

} // namespace MetricSpaceExtensions
