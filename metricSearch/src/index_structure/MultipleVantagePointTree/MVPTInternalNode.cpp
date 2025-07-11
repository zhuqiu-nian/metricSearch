// MVPTInternalNode.cpp
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTInternalNode.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include <algorithm>

MVPTInternalNode::MVPTInternalNode(
    std::vector<DataPtr> pivots,
    std::vector<std::unique_ptr<MVPTNode>> children,
    std::vector<std::vector<long double>> lowerBounds,
    std::vector<std::vector<long double>> upperBounds,
    std::shared_ptr<MetricDistance> dist)
    : pivots_(std::move(pivots)),
    children_(std::move(children)),
    lowerBounds_(std::move(lowerBounds)),
    upperBounds_(std::move(upperBounds)),
    dist_(std::move(dist)) {}

std::vector<DataPtr> MVPTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    // 检查所有支撑点是否在范围内
    for (const auto& vp : pivots_) {
        long double d = dist_->distance(q, *vp);
        if (distanceCount) (*distanceCount)++;
        if (d <= r) {
            result.push_back(vp);
        }
    }

    // 对于每棵子树尝试剪枝
    for (size_t ci = 0; ci < children_.size(); ++ci) {
        bool done = false;

        // 检查是否有任何一个支撑点可以完全包含该子树
        for (size_t pi = 0; pi < pivots_.size(); ++pi) {
            long double d_vp_q = dist_->distance(q, *pivots_[pi]);
            if (distanceCount) (*distanceCount)++;

            if (d_vp_q + upperBounds_[pi][ci] <= r) {
                // 子树全部满足条件，加入结果集
                auto childResult = children_[ci]->getAll();
                result.insert(result.end(), childResult.begin(), childResult.end());
                done = true;
                break;
            }

            if (d_vp_q + r <= lowerBounds_[pi][ci] ||
                d_vp_q - r >= upperBounds_[pi][ci]) {
                // 子树不可能有解，跳过
                done = true;
                break;
            }
        }

        if (!done) {
            // 无法剪枝，递归搜索
            auto childResult = children_[ci]->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
    }

    return result;
}

DataList MVPTInternalNode::getAll() const {
    DataList all;
    for (const auto& p : pivots_) {
        all.push_back(p);
    }
    for (const auto& child : children_) {
        auto childAll = child->getAll();
        all.insert(all.end(), childAll.begin(), childAll.end());
    }
    return all;
}