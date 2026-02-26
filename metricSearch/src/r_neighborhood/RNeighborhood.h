#ifndef R_NEIGHBORHOOD_H
#define R_NEIGHBORHOOD_H

#include <vector>
#include <memory>
#include <string>
#include <array>
#include "../include/interfaces/MetricData.h"
#include "../include/interfaces/MetricDistance.h"
#include "../src/PivotSelector/PivotSelector.h"

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

struct Hyperplane {
    long double a, b, c, d;  // ax + by + cz + d = 0
    Hyperplane(long double a_, long double b_, long double c_, long double d_)
        : a(a_), b(b_), c(c_), d(d_) {}
};

// 新：传入查询半径 queryRadius，内部为每个平面计算动态宽度
long long countPointsInRNeighborhood(
    const std::vector<std::array<long double, 3>>& pivotSpacePoints,
    const std::vector<Hyperplane>& hyperplanes,
    long double queryRadius  // e.g., 0.02
);

std::vector<Hyperplane> extractHyperplanesFromMVPT(
    const DataList& data,
    int k, int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method
);

std::vector<Hyperplane> extractHyperplanesFromCGHT(
    const DataList& data,
    int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method
);

std::vector<Hyperplane> extractHyperplanesFromCLPT(
    const DataList& data,
    int k, int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method,
    const std::string& normalStrategy
);

#endif // R_NEIGHBORHOOD_H