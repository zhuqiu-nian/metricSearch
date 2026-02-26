// VPTInternalNode.h
#ifndef VPTINTERNALNODE_H
#define VPTINTERNALNODE_H

#include "VPTNode.h"

class VPTInternalNode : public VPTNode {
public:
    VPTInternalNode(DataPtr vp,
        long double radius,
        std::unique_ptr<VPTNode> left,
        std::unique_ptr<VPTNode> right,
        std::shared_ptr<MetricDistance> dist);

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) ;
    DataList getAll() const override;

private:
    DataPtr vp_;
    long double splitRadius_;
    std::unique_ptr<VPTNode> left_, right_;
    std::shared_ptr<MetricDistance> dist_;
};

#endif // VPTINTERNALNODE_H