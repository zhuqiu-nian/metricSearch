#include "../../../include/core/Distance_subclass/LonePointDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"
#include <cmath>

long double LonePointDistance::distance(const MetricData& a, const MetricData& b) const {
    // 动态类型转换确保数据可比较
    const auto& vecA = dynamic_cast<const VectorData&>(a).getVector();
    const auto& vecB = dynamic_cast<const VectorData&>(b).getVector();

    // 维度检查
    if (vecA.size() != vecB.size()) {
        throw std::runtime_error("向量维度不匹配");
    }

    // 判断所有维度是否严格相等
    for (size_t i = 0; i < vecA.size(); ++i) {
        if (std::abs(vecA[i] - vecB[i]) > 1e-9) {  // 考虑浮点误差
            return 1.0;  // 不相等
        }
    }
    return 0.0;  // 相等
}

std::string LonePointDistance::getName() const {
    return "孤点距离 (0-1离散距离)";
}