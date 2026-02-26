// CLPPartitioner.h
#ifndef CLPPARTITIONER_H
#define CLPPARTITIONER_H

#include "../../include/interfaces/MetricData.h"
#include "../../include/interfaces/MetricDistance.h"
#include <vector>
#include <memory>
#include <limits>

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

// === 单层划分结果 ===
struct SingleLevelPartitionResult {
    std::vector<DataList> subDataList;      // f 个子数据集
    std::vector<double> lowerBounds;        // 每个子空间在当前法向量上的下界
    std::vector<double> upperBounds;        // 上界
};

class CLPPartitioner {
public:
    // === 新增：单层划分接口（用于递归建树）===
    static SingleLevelPartitionResult partitionSingleLevel(
        const std::vector<std::vector<double>>& dataCoordinates,
        const std::vector<double>& normalVector,
        int numPartitions
    );

    // === 原有接口保留（用于查询时的排除率评估，可选）===
    static constexpr double QR = 0.1;

    static std::vector<std::vector<double>> getCordinate(MetricDistance* metric, const std::vector<DataPtr>& pivots, const DataList& data, long long* distCount);

    static std::vector<std::vector<double>> getCGHNormalVectors(int dim);
    static std::vector<std::vector<double>> getVPNormalVectors(int dim);
    static std::vector<std::vector<double>> getPCANormalVectors(const std::vector<std::vector<double>>& dataCordinates);
    static std::vector<std::vector<double>> getBallPlaneNormalVectors(int n, double radius);

private:
    struct DoubleWrapper {
        double projection;
        size_t originalIndex;
        DoubleWrapper(double p, size_t idx) : projection(p), originalIndex(idx) {}
    };
};

#endif // CLPPARTITIONER_H