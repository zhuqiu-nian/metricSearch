#pragma once
#include "../../interfaces/MetricDistance.h"
#include <memory>

class LonePointDistance : public MetricDistance {
public:
    // 计算孤点距离（相等为0，不等为1）
    long double distance(const MetricData& a, const MetricData& b) const override;

    // 返回距离函数名称
    std::string getName() const override;
};