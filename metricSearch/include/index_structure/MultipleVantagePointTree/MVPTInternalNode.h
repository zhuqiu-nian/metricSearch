// MVPTInternalNode.h
#ifndef MVPTINTERNALNODE_H
#define MVPTINTERNALNODE_H

#include "MVPTNode.h"
#include <vector>
#include <memory>

class MetricDistance;

class MVPTInternalNode : public MVPTNode {
public:
    MVPTInternalNode(std::vector<DataPtr> pivots,
        std::vector<std::unique_ptr<MVPTNode>> children,
        std::vector<std::vector<long double>> lowerBounds,
        std::vector<std::vector<long double>> upperBounds,
        std::shared_ptr<MetricDistance> dist);

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    std::vector<DataPtr> pivots_;
    std::vector<std::unique_ptr<MVPTNode>> children_;
    std::vector<std::vector<long double>> lowerBounds_; // [pivots][children]
    std::vector<std::vector<long double>> upperBounds_;
    std::shared_ptr<MetricDistance> dist_;
};

#endif // MVPTINTERNALNODE_H
