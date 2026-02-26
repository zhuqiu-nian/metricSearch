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

std::vector<DataPtr> VPTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    long double d_vq = dist_->distance(q, *vp_);
    if (distanceCount) (*distanceCount)++;  // 统计一次距离计算

    // 1. 检查支撑点是否在查询范围内
    if (d_vq <= r) {
        result.push_back(vp_);
    }

    // 2. 处理左子树（内部点：d(vp, x) <= splitRadius_）
    if (d_vq + splitRadius_ <= r) {
        //完全包含：左子树所有点都在查询球内
        DataList leftAll = left_->getAll();
        result.insert(result.end(), leftAll.begin(), leftAll.end());
    }
    else if (d_vq <= splitRadius_ + r) {
        // 部分重叠：需要递归搜索
        auto leftResults = left_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), leftResults.begin(), leftResults.end());
    }
    // else: 完全不相交，跳过

    // 3. 处理右子树（外部点：d(vp, x) > splitRadius_）
    if (d_vq >= splitRadius_ - r) {
        // 注意：原条件 "d_vq + r >= splitRadius_" 等价于 "d_vq >= splitRadius_ - r"
        // 但这里无法做“完全包含”优化（因为右子树无上界），只能剪枝或递归
        auto rightResults = right_->rangeSearch(q, r, distanceCount);
        result.insert(result.end(), rightResults.begin(), rightResults.end());
    }
    // else: 完全不相交（d_vq + r < splitRadius_ ⇒ 所有右子树点离 q 太远），跳过

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