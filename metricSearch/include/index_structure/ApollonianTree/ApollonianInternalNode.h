// ApollonianInternalNode.h
#ifndef APOLLONIAN_INTERNAL_NODE_H
#define APOLLONIAN_INTERNAL_NODE_H

#include "ApollonianNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include <memory>
#include <vector>

class ApollonianInternalNode : public ApollonianNode {
public:
    ApollonianInternalNode(
        DataPtr c1,
        DataPtr c2,
        long double c1_ratio,   // c1 < 1
        long double c2_ratio,   // c2 > 1
        std::unique_ptr<ApollonianNode> leftChild,
        std::unique_ptr<ApollonianNode> midChild,
        std::unique_ptr<ApollonianNode> rightChild,
        std::shared_ptr<MetricDistance> dist
    );

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    DataPtr c1_, c2_;
    long double c1_ratio_, c2_ratio_;
    std::unique_ptr<ApollonianNode> left_, mid_, right_;
    std::shared_ptr<MetricDistance> dist_;

    // Helper: check if child can be pruned or fully included
    enum class SearchAction { SEARCH, SKIP, FULL };
    SearchAction decideAction(long double a, long double b, long double r, bool isLeft) const;
};

#endif