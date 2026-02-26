#include "RNeighborhood.h"
#include "../include/utils/MetricSpaceSearch.h"
#include "../include/utils/CLPPartitioner.h"  
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>

// 辅助：计算点到平面的距离
long double pointToPlaneDistance(
    const std::array<long double, 3>& pt,
    const Hyperplane& plane)
{
    long double num = std::abs(plane.a * pt[0] + plane.b * pt[1] + plane.c * pt[2] + plane.d);
    long double den = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    if (den == 0) return std::numeric_limits<long double>::infinity();
    return num / den;
}

// 核心修改：使用动态宽度
long long countPointsInRNeighborhood(
    const std::vector<std::array<long double, 3>>& points,
    const std::vector<Hyperplane>& planes,
    long double queryRadius)
{
    long long count = 0;
    for (const auto& pt : points) {
        bool inAnyNeighborhood = false;
        for (const auto& plane : planes) {
            // 计算该平面的 r-neighborhood 半宽
            long double l1_norm = std::abs(plane.a) + std::abs(plane.b) + std::abs(plane.c);
            long double l2_norm = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
            if (l2_norm == 0) continue;

            long double halfWidth = (2.0L * queryRadius * l1_norm) / l2_norm;

            // 判断点到平面距离是否 ≤ halfWidth
            if (pointToPlaneDistance(pt, plane) <= halfWidth) {
                inAnyNeighborhood = true;
                break;
            }
        }
        if (inAnyNeighborhood) {
            ++count;
        }
    }
    return count;
}

// ====== MVPT 超平面提取（不变）======
std::vector<Hyperplane> extractHyperplanesFromMVPT(
    const DataList& data,
    int k, int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method)
{
    if (k != 3 || f != 3) {
        throw std::invalid_argument("Only k=3, f=3 supported.");
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    std::vector<int> pivotIndices = PivotSelector::selectPivots(data, k, dist, method, 0.35);
    std::vector<DataPtr> vps;
    for (int idx : pivotIndices) vps.push_back(data[idx]);

    std::vector<DataPtr> restData;
    for (const auto& item : data) {
        if (std::find(vps.begin(), vps.end(), item) == vps.end()) {
            restData.push_back(item);
        }
    }

    if (restData.empty()) return {};

    std::vector<long double> d0, d1, d2;
    for (const auto& item : restData) {
        d0.push_back(dist->distance(*vps[0], *item));
        d1.push_back(dist->distance(*vps[1], *item));
        d2.push_back(dist->distance(*vps[2], *item));
    }

    std::sort(d0.begin(), d0.end());
    std::sort(d1.begin(), d1.end());
    std::sort(d2.begin(), d2.end());

    size_t n = d0.size();
    long double t0_1 = d0[(n * 1) / 3];
    long double t0_2 = d0[(n * 2) / 3];
    long double t1_1 = d1[(n * 1) / 3];
    long double t1_2 = d1[(n * 2) / 3];
    long double t2_1 = d2[(n * 1) / 3];
    long double t2_2 = d2[(n * 2) / 3];

    return {
        {1.0, 0.0, 0.0, -t0_1},
        {1.0, 0.0, 0.0, -t0_2},
        {0.0, 1.0, 0.0, -t1_1},
        {0.0, 1.0, 0.0, -t1_2},
        {0.0, 0.0, 1.0, -t2_1},
        {0.0, 0.0, 1.0, -t2_2}
    };
}

// ====== CGHT 超平面提取（不变）======
std::vector<Hyperplane> extractHyperplanesFromCGHT(
    const DataList& data,
    int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method)
{
    if (f != 3) {
        throw std::invalid_argument("Only f=3 supported.");
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    std::vector<int> pivotIndices = PivotSelector::selectPivots(data, 3, dist, method, 0.35);
    DataPtr p1 = data[pivotIndices[0]];
    DataPtr p2 = data[pivotIndices[1]];
    DataPtr p3 = data[pivotIndices[2]];

    std::vector<long double> f1, f2, f3;
    for (const auto& item : data) {
        long double d1 = dist->distance(*p1, *item);
        long double d2 = dist->distance(*p2, *item);
        long double d3 = dist->distance(*p3, *item);
        f1.push_back(d1 + d2 - d3);
        f2.push_back(d1 - d2 + d3);
        f3.push_back(d1 - d2 - d3);
    }

    std::sort(f1.begin(), f1.end());
    std::sort(f2.begin(), f2.end());
    std::sort(f3.begin(), f3.end());

    size_t n = f1.size();
    long double th1_1 = f1[(n * 1) / 3];
    long double th1_2 = f1[(n * 2) / 3];
    long double th2_1 = f2[(n * 1) / 3];
    long double th2_2 = f2[(n * 2) / 3];
    long double th3_1 = f3[(n * 1) / 3];
    long double th3_2 = f3[(n * 2) / 3];

    return {
        {1.0, 1.0, -1.0, -th1_1},
        {1.0, 1.0, -1.0, -th1_2},
        {1.0, -1.0, 1.0, -th2_1},
        {1.0, -1.0, 1.0, -th2_2},
        {1.0, -1.0, -1.0, -th3_1},
        {1.0, -1.0, -1.0, -th3_2}
    };
}

// ====== CLPT 超平面提取 ======

std::vector<Hyperplane> extractHyperplanesFromCLPT(
    const DataList& data,
    int k, int f,
    int distanceType, int dataType,
    PivotSelector::SelectionMethod method,
    const std::string& normalStrategy)
{
    if (k != 3 || f != 3) {
        throw std::invalid_argument("Only k=3, f=3 supported for fair comparison.");
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    std::vector<int> pivotIndices = PivotSelector::selectPivots(data, k, dist, method, 0.35);
    std::vector<DataPtr> pivots;
    for (int idx : pivotIndices) pivots.push_back(data[idx]);

    // 构建 restData（排除 pivots）
    std::vector<DataPtr> restData;
    for (const auto& item : data) {
        if (std::find(pivots.begin(), pivots.end(), item) == pivots.end()) {
            restData.push_back(item);
        }
    }

    if (restData.empty()) return {};

    // Step 1: 获取 pivot space 坐标 (as double)
    std::vector<std::vector<double>> coords;
    for (const auto& item : restData) {
        std::vector<double> pt(k);
        for (int i = 0; i < k; ++i) {
            pt[i] = static_cast<double>(dist->distance(*pivots[i], *item));
        }
        coords.push_back(pt);
    }

    // Step 2: 生成法向量
    std::vector<std::vector<double>> normalVectors;
    if (normalStrategy == "CGH") {
        normalVectors = CLPPartitioner::getCGHNormalVectors(k);
    }
    else if (normalStrategy == "PCA") {
        normalVectors = CLPPartitioner::getPCANormalVectors(coords);
        // Ensure we have at least k vectors (repeat last if needed)
        while (normalVectors.size() < static_cast<size_t>(k)) {
            normalVectors.push_back(normalVectors.back());
        }
    }
    else if (normalStrategy == "VP") {
        auto vpVecs = CLPPartitioner::getVPNormalVectors(k);
        normalVectors = vpVecs;
        while (normalVectors.size() < static_cast<size_t>(k)) {
            normalVectors.push_back(vpVecs[0]);
        }
    }
    else {
        normalVectors = CLPPartitioner::getCGHNormalVectors(k); // default
    }

    if (normalVectors.size() < static_cast<size_t>(k)) {
        throw std::runtime_error("Failed to generate enough normal vectors for CLPT.");
    }

    std::vector<Hyperplane> planes;

    // 对每一层（每个法向量）
    for (int layer = 0; layer < k; ++layer) {
        const auto& normal = normalVectors[layer]; // size = k

        // 投影到当前法向量
        std::vector<std::pair<double, size_t>> projections;
        for (size_t i = 0; i < coords.size(); ++i) {
            double proj = 0.0;
            for (int d = 0; d < k; ++d) {
                proj += coords[i][d] * normal[d];
            }
            projections.emplace_back(proj, i);
        }

        std::sort(projections.begin(), projections.end());

        size_t n = projections.size();
        double t1 = projections[(n * 1) / 3].first;
        double t2 = projections[(n * 2) / 3].first;

        // 超平面: a*x + b*y + c*z - t = 0 → d = -t
        planes.emplace_back(
            static_cast<long double>(normal[0]),
            static_cast<long double>(normal[1]),
            static_cast<long double>(normal[2]),
            static_cast<long double>(-t1)
        );
        planes.emplace_back(
            static_cast<long double>(normal[0]),
            static_cast<long double>(normal[1]),
            static_cast<long double>(normal[2]),
            static_cast<long double>(-t2)
        );
    }

    return planes;
}