// VPTInternalNode.cpp
#include "../../../include/index_structure/VantagePointTree/VPTInternalNode.h"
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"

VPTInternalNode::VPTInternalNode(DataPtr vp,
    long double radius,
    std::unique_ptr<VPTNode> left,
    std::unique_ptr<VPTNode> right,
    std::shared_ptr<MetricDistance> dist)
    : vp_(vp), splitRadius_(radius), left_(std::move(left)), right_(std::move(right)), dist_(dist) {}

std::vector<DataPtr> VPTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount)  {
    std::vector<DataPtr> result;

    long double d_vq = dist_->distance(q, *vp_);
    if (distanceCount) (*distanceCount)++;  // Õ≥º∆“ª¥Œæ‡¿Îº∆À„

    if (d_vq <= r) {
        result.push_back(vp_);
    }

    // ◊Û◊” ˜£∫æ‡¿Î °‹ splitRadius + r
    if (d_vq <= splitRadius_ + r) {
        auto leftResults = left_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), leftResults.begin(), leftResults.end());
    }

    // ”“◊” ˜£∫æ‡¿Î > splitRadius - r
    if (d_vq + r >= splitRadius_) {
        auto rightResults = right_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), rightResults.begin(), rightResults.end());
    }

    return result;
}

DataList VPTInternalNode::getAll() const {
    DataList leftAll = left_->getAll();
    DataList rightAll = right_->getAll();

    DataList all;
    all.reserve(leftAll.size() + rightAll.size() + 1);
    all.push_back(vp_);
    all.insert(all.end(), leftAll.begin(), leftAll.end());
    all.insert(all.end(), rightAll.begin(), rightAll.end());

    return all;
}