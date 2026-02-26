#pragma once

#include "GHTNode.h"
#include "../../interfaces/MetricDistance.h"

class GHTInternalNode : public GHTNode {
public:
    GHTInternalNode(DataPtr c1, DataPtr c2,
        std::unique_ptr<GHTNode> left,
        std::unique_ptr<GHTNode> right,
        std::shared_ptr<MetricDistance> dist);

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;

private:
    DataPtr c1_, c2_;
    std::unique_ptr<GHTNode> left_, right_;
    std::shared_ptr<MetricDistance> dist_;  // ±£´æ¾àÀëº¯Êý
};