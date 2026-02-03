// ImageFeatureDistance.h
#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../Data_subclass/ImageFeatureData.h"

class ImageFeatureDistance : public MetricDistance {
public:
    long double distance(const MetricData& a, const MetricData& b) const override;
    std::string getName() const override;
};