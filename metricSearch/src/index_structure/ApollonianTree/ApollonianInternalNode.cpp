// ApollonianInternalNode.cpp
#include "../../../include/index_structure/ApollonianTree/ApollonianInternalNode.h"
#include <limits>
#include <cmath>

ApollonianInternalNode::ApollonianInternalNode(
    DataPtr c1,
    DataPtr c2,
    long double c1_ratio,
    long double c2_ratio,
    std::unique_ptr<ApollonianNode> leftChild,
    std::unique_ptr<ApollonianNode> midChild,
    std::unique_ptr<ApollonianNode> rightChild,
    std::shared_ptr<MetricDistance> dist)
    : c1_(std::move(c1)), c2_(std::move(c2)),
    c1_ratio_(c1_ratio), c2_ratio_(c2_ratio),
    left_(std::move(leftChild)),
    mid_(std::move(midChild)),
    right_(std::move(rightChild)),
    dist_(std::move(dist))
{}

DataList ApollonianInternalNode::getAll() const {
    DataList all = { c1_, c2_ };
    if (left_) { auto l = left_->getAll();  all.insert(all.end(), l.begin(), l.end()); }
    if (mid_) { auto m = mid_->getAll();   all.insert(all.end(), m.begin(), m.end()); }
    if (right_) { auto r = right_->getAll(); all.insert(all.end(), r.begin(), r.end()); }
    return all;
}

// Helper to classify search action for a subtree
ApollonianInternalNode::SearchAction ApollonianInternalNode::decideAction(
    long double a, long double b, long double r, bool isLeft) const
{
    if (isLeft) {
        // Check for ALL-L and EXCLUDE-L
        if (c1_ratio_ * b - a > (c1_ratio_ + 1) * r) {
            return SearchAction::FULL;   // ALL-L
        }
        if (a - c1_ratio_ * b >= (c1_ratio_ + 1) * r) {
            return SearchAction::SKIP;   // EXCLUDE-L
        }
    }
    else {
        // isRight: check ALL-R and EXCLUDE-R
        if (a - c2_ratio_ * b > (c2_ratio_ + 1) * r) {
            return SearchAction::FULL;   // ALL-R
        }
        if (c2_ratio_ * b - a >= (c2_ratio_ + 1) * r) {
            return SearchAction::SKIP;   // EXCLUDE-R
        }
    }
    return SearchAction::SEARCH;
}

std::vector<DataPtr> ApollonianInternalNode::rangeSearch(
    const MetricData& q, long double r, long long* distanceCount)
{
    std::vector<DataPtr> result;

    // Step 1: Compute a = d(q, C1), b = d(q, C2)
    long double a = dist_->distance(q, *c1_);
    long double b = dist_->distance(q, *c2_);
    if (distanceCount) (*distanceCount) += 2;

    // Add C1, C2 if they satisfy query
    if (a <= r) result.push_back(c1_);
    if (b <= r) result.push_back(c2_);

    // Step 2: Check middle region containment (optional optimization)
    bool excludeLeft = (a - c1_ratio_ * b >= (c1_ratio_ + 1) * r);
    bool excludeRight = (c2_ratio_ * b - a >= (c2_ratio_ + 1) * r);
    bool allInMiddle = excludeLeft && excludeRight;

    // Left child
    if (left_ && !allInMiddle) {
        auto action = decideAction(a, b, r, true);
        if (action == SearchAction::FULL) {
            auto full = left_->getAll();
            result.insert(result.end(), full.begin(), full.end());
        }
        else if (action == SearchAction::SEARCH) {
            auto res = left_->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), res.begin(), res.end());
        }
        // else SKIP ¡ú do nothing
    }

    // Mid child: always search unless we know it's empty (hard to prune M)
    if (mid_ && !allInMiddle) {
        // M is hard to prune; but if both sides are excluded, we could skip L/R and only do M
        // However, here we just search M normally
        auto res = mid_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), res.begin(), res.end());
    }
    else if (mid_ && allInMiddle) {
        // Strong case: everything is in M
        auto full = mid_->getAll();
        result.insert(result.end(), full.begin(), full.end());
    }

    // Right child
    if (right_ && !allInMiddle) {
        auto action = decideAction(a, b, r, false);
        if (action == SearchAction::FULL) {
            auto full = right_->getAll();
            result.insert(result.end(), full.begin(), full.end());
        }
        else if (action == SearchAction::SEARCH) {
            auto res = right_->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), res.begin(), res.end());
        }
    }

    return result;
}