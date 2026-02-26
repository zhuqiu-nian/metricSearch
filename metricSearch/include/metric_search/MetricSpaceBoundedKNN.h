#ifndef METRICSPACEBOUNDED_KNN_H
#define METRICSPACEBOUNDED_KNN_H

#include "../../include/utils/MetricSpaceSearch.h"  // 包含 SearchResult 等定义
#include <vector>
#include <memory>

static MetricSpaceSearch::SearchResult boundedKnnQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        int k,
        const std::shared_ptr<MetricData>& pivot = nullptr,
        long double radius = numeric_limits<long double>::max());

void runBoundedKnnQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType);


#endif // METRICSPACEBOUNDED_KNN_H
