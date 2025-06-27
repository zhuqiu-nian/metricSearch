#ifndef METRICSPACEKNN_H
#define METRICSPACEKNN_H

#include "../../include/utils/MetricSpaceSearch.h"  // 包含 SearchResult 等定义
#include <vector>
#include <memory>


static MetricSpaceSearch::SearchResult knnQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        int k,
        const std::shared_ptr<MetricData>& pivot = nullptr);

void runKnnQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType);


#endif // METRICSPACEKNN_H
