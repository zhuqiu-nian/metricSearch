#pragma once
#include "../../interfaces/MetricDistance.h"
#include  <string>

// 欧几里得距离
class EuclideanDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;
    string getName() const override;
};