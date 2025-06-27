#pragma once

#include "../../interfaces/MetricData.h"
#include <vector>
#include <memory>

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

//³éÏó»ùÀà GHTNode
class GHTNode {
public:
    virtual ~GHTNode() = default;
    virtual std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) = 0;
};