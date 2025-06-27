// VPTNode.h
#ifndef VPTNODE_H
#define VPTNODE_H

#include <vector>
#include <memory>
#include "../../interfaces/MetricData.h"
#include "../../interfaces/MetricDistance.h"

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

class VPTNode {
public:
    virtual ~VPTNode() = default;
    virtual std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) = 0;
    virtual DataList getAll() const = 0;
};

#endif // VPTNODE_H