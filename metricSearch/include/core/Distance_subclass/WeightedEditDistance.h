#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../../../include/core/Data_subclass/ProteinData.h"
#include <unordered_map>



long double getCost(char a, char b);

class WeightedEditDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;
    std::string getName() const override {
        return "WeightedEditDistance";
    }
};
