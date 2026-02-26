// CLPPartitioner.cpp
#include "../../include/utils/CLPPartitioner.h"
#include <algorithm>
#include <cmath>
#include <cassert>

// ==================== 工具函数 ====================

std::vector<std::vector<double>> CLPPartitioner::getCordinate(MetricDistance* metric, const std::vector<DataPtr>& pivots, const DataList& data, long long* distCount) {
    size_t n = data.size();
    int k = static_cast<int>(pivots.size());
    if (k == 0 || n == 0) return {};
    std::vector<std::vector<double>> coords(n, std::vector<double>(k));
    for (size_t i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            coords[i][j] = metric->distance(*data[i], *pivots[j]);
            if (distCount) (*distCount)++;
        }
    }
    return coords;
}

// CGH: 坐标轴方向（最符合“完全线性划分”定义）
std::vector<std::vector<double>> CLPPartitioner::getCGHNormalVectors(int dim) {
    if (dim <= 0) return {};
    std::vector<std::vector<double>> res;
    for (int i = 0; i < dim; ++i) {
        std::vector<double> v(dim, 0.0);
        v[i] = 1.0;
        res.push_back(v);
    }
    return res;
}

// VP: 对角线方向
std::vector<std::vector<double>> CLPPartitioner::getVPNormalVectors(int dim) {
    if (dim <= 0) return {};
    std::vector<double> v(dim, 1.0 / std::sqrt(static_cast<double>(dim)));
    return { v };
}

// 简化版 PCA
std::vector<std::vector<double>> CLPPartitioner::getPCANormalVectors(const std::vector<std::vector<double>>& dataCordinates) {
    if (dataCordinates.empty()) return {};
    int dim = static_cast<int>(dataCordinates[0].size());
    if (dim <= 1) return {};

    std::vector<double> mean(dim, 0.0);
    for (const auto& pt : dataCordinates) {
        for (int i = 0; i < dim; ++i) mean[i] += pt[i];
    }
    double n = static_cast<double>(dataCordinates.size());
    for (double& m : mean) m /= n;

    std::vector<std::vector<double>> cov(dim, std::vector<double>(dim, 0.0));
    for (const auto& pt : dataCordinates) {
        std::vector<double> centered(dim);
        for (int i = 0; i < dim; ++i) centered[i] = pt[i] - mean[i];
        for (int i = 0; i < dim; ++i)
            for (int j = 0; j < dim; ++j)
                cov[i][j] += centered[i] * centered[j];
    }

    std::vector<double> vec(dim, 1.0);
    for (int iter = 0; iter < 20; ++iter) {
        std::vector<double> newVec(dim, 0.0);
        for (int i = 0; i < dim; ++i)
            for (int j = 0; j < dim; ++j)
                newVec[i] += cov[i][j] * vec[j];
        double norm = 0.0;
        for (double v : newVec) norm += v * v;
        if (norm == 0) break;
        norm = std::sqrt(norm);
        for (double& v : newVec) v /= norm;
        vec = newVec;
    }
    return { vec };
}

// 球面采样（仅 dim=3）
std::vector<std::vector<double>> CLPPartitioner::getBallPlaneNormalVectors(int n, double radius) {
    if (n <= 0) return {};
    std::vector<std::vector<double>> dirs = { {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
    std::vector<std::vector<double>> res;
    for (auto d : dirs) {
        double norm = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        if (norm > 0) {
            for (double& x : d) x /= norm;
            res.push_back(d);
        }
    }
    return res;
}

// ==================== 核心：单层划分 ====================

SingleLevelPartitionResult CLPPartitioner::partitionSingleLevel(
    const std::vector<std::vector<double>>& dataCoordinates,
    const std::vector<double>& normalVector,
    int numPartitions)
{
    assert(!dataCoordinates.empty());
    assert(normalVector.size() == dataCoordinates[0].size());
    assert(numPartitions >= 2);

    size_t dataSize = dataCoordinates.size();
    int dim = static_cast<int>(normalVector.size());

    // Step 1: 计算每个点在法向量上的投影
    std::vector<DoubleWrapper> projections;
    projections.reserve(dataSize);
    for (size_t i = 0; i < dataSize; ++i) {
        double proj = 0.0;
        for (int d = 0; d < dim; ++d) {
            proj += dataCoordinates[i][d] * normalVector[d];
        }
        projections.emplace_back(proj, i);
    }

    // Step 2: 按投影值排序
    std::sort(projections.begin(), projections.end(),
        [](const DoubleWrapper& a, const DoubleWrapper& b) {
            return a.projection < b.projection;
        });

    // Step 3: 均匀分割为 numPartitions 个桶
    std::vector<DataList> subDataList(numPartitions);
    std::vector<double> lowerBounds(numPartitions, 0.0);
    std::vector<double> upperBounds(numPartitions, 0.0);

    // 注意：这里我们不返回原始数据，只返回索引映射
    // 实际数据映射在 bulkLoad 中完成
    size_t bucketSize = (dataSize + numPartitions - 1) / numPartitions; // ceil division

    for (int i = 0; i < numPartitions; ++i) {
        size_t startIdx = i * bucketSize;
        size_t endIdx = std::min(startIdx + bucketSize, dataSize);

        if (startIdx >= dataSize) {
            // 空桶
            lowerBounds[i] = 0.0;
            upperBounds[i] = 0.0;
            continue;
        }

        lowerBounds[i] = projections[startIdx].projection;
        upperBounds[i] = projections[endIdx - 1].projection;
    }

    return { {}, lowerBounds, upperBounds }; // subDataList 在 bulkLoad 中填充
}