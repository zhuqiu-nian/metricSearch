// MVPTNode.h
#ifndef MVPTNODE_H
#define MVPTNODE_H

#include <vector>
#include <memory>
#include "../../interfaces/MetricData.h"
#include "../../interfaces/MetricDistance.h"

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

class MVPTNode {
public:
    virtual ~MVPTNode() = default;
    virtual std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) = 0;
    virtual DataList getAll() const = 0; // 支持剪枝时获取所有子树数据
};

#endif // MVPTNODE_H