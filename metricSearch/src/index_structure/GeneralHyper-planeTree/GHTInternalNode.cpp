#include "../../../include/index_structure/GeneralHyper-planeTree/GHTInternalNode.h"

GHTInternalNode::GHTInternalNode(DataPtr c1, DataPtr c2,
    std::unique_ptr<GHTNode> left,
    std::unique_ptr<GHTNode> right,
    std::shared_ptr<MetricDistance> dist)
    : c1_(std::move(c1)), c2_(std::move(c2)),
    left_(std::move(left)), right_(std::move(right)),
    dist_(std::move(dist)) {}

std::vector<DataPtr> GHTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    long double d_c1 = dist_->distance(q, *c1_);
    long double d_c2 = dist_->distance(q, *c2_);
    if (distanceCount) (*distanceCount)+=2;  // 统计一次距离计算

    if (d_c1 <= r) result.push_back(c1_);
    if (d_c2 <= r) result.push_back(c2_);

    if (d_c1 - d_c2 <= 2 * r) {
        auto leftResult = left_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), leftResult.begin(), leftResult.end());
    }

    if (d_c2 - d_c1 < 2 * r) {
        auto rightResult = right_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), rightResult.begin(), rightResult.end());
    }

    return result;
}