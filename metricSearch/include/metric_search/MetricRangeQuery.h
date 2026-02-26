#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "../interfaces/MetricData.h"
#include "../interfaces/MetricDistance.h"
#include "../utils/MetricSpaceSearch.h"

namespace MetricSpaceExtensions {

    // 基于三角不等式的度量空间范围查询
    MetricSpaceSearch::SearchResult rangeQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const std::shared_ptr<MetricData>& pivot = nullptr);

    void runRangeQuery(
        const vector<shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

} // namespace MetricSpaceExtensions
