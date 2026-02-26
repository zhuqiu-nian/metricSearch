#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../../../include/core/Data_subclass/StringData.h"
#include <vector>
#include <algorithm>

class EditDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;
    std::string getName() const override { return "±‡º≠æ‡¿Î"; }
};
