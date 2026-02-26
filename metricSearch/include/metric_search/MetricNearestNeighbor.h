#ifndef METRICSPACE_NNQUERY_H
#define METRICSPACE_NNQUERY_H

#include <vector>
#include <memory>
#include "../interfaces/MetricData.h"
#include "../interfaces/MetricDistance.h"
#include "../utils/MetricSpaceSearch.h"

class MetricSpaceNNQuery {
public:
    static MetricSpaceSearch::SearchResult nearestNeighbor(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        const std::shared_ptr<MetricData>& pivot);
};

// 交互入口函数声明
void runNearestNeighborQuery(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType);

#endif // METRICSPACE_NNQUERY_H
