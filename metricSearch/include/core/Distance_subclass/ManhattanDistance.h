#pragma once
#include "../../interfaces/MetricDistance.h"

class ManhattanDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;
    std::string getName() const override { return "Âü¹ş¶Ù¾àÀë"; }
};
