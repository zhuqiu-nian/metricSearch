#pragma once
#include "../../interfaces/MetricDistance.h"
#include  <string>

// ChebyshevDistancce���뺯��
class ChebyshevDistance : public MetricDistance {
public:
    double distance(const MetricData& a, const MetricData& b) const override;
    string getName() const override;
};
