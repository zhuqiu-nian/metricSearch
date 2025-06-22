#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../../../include/core/Data_subclass/StringData.h"

class HammingDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;

    std::string getName() const override {
        return "º£Ã÷¾àÀë";
    }
};
