#include "../../../include/core/Distance_subclass/EuclideanDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"

// EuclideanDistance 实现
long double EuclideanDistance::distance(const MetricData& a, const MetricData& b) const {
    const auto& va = dynamic_cast<const VectorData&>(a).getVector();
    const auto& vb = dynamic_cast<const VectorData&>(b).getVector();

    if (va.size() != vb.size()) {
        throw runtime_error("向量维度不匹配");
    }

    long double sum = 0.0;
    for (size_t i = 0; i < va.size(); ++i) {
        long double diff = va[i] - vb[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

string EuclideanDistance::getName() const {
    return "欧几里得距离";
}