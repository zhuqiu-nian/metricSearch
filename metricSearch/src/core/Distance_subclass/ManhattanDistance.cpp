#include "../../../include/core/Distance_subclass/ManhattanDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"
#include <cmath>

long double ManhattanDistance::distance(const MetricData& a, const MetricData& b) const {
    const auto& vecA = dynamic_cast<const VectorData&>(a).getVector();
    const auto& vecB = dynamic_cast<const VectorData&>(b).getVector();

    if (vecA.size() != vecB.size()) {
        throw std::runtime_error("向量维度不匹配");
    }

    long double sum = 0.0;
    for (size_t i = 0; i < vecA.size(); ++i) {
        sum += std::abs(vecA[i] - vecB[i]);
    }
    return sum;
}