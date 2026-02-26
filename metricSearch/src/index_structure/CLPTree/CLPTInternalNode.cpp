#include "../../../include/index_structure/CLPTree/CLPTInternalNode.h"
#include <algorithm>
#include <iostream> // For debugging if needed

CLPTInternalNode::CLPTInternalNode(
    std::vector<DataPtr> pivots,
    std::vector<std::unique_ptr<CLPTNode>> children,
    std::vector<std::vector<double>> normalVectors,
    std::vector<std::vector<double>> lowerBounds,
    std::vector<std::vector<double>> upperBounds,
    std::vector<std::vector<double>> longestDistancesToPivots,
    std::shared_ptr<MetricDistance> dist
)
    : pivots_(std::move(pivots)),
    children_(std::move(children)),
    normalVectors_(std::move(normalVectors)),
    lowerBounds_(std::move(lowerBounds)),
    upperBounds_(std::move(upperBounds)),
    longestDistancesToPivots_(std::move(longestDistancesToPivots)),
    dist_(std::move(dist)) {}

std::vector<DataPtr> CLPTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    // Step 1: 预计算 q 到所有 pivots 的距离
    std::vector<long double> distQToPivots;
    distQToPivots.reserve(pivots_.size());
    for (const auto& vp : pivots_) {
        long double d = dist_->distance(q, *vp);
        if (distanceCount) (*distanceCount)++;
        distQToPivots.push_back(d);

        if (d <= r) {
            result.push_back(vp);
        }
    }

    size_t numChildren = children_.size();
    size_t numDimensions = normalVectors_.size();

    // 安全检查：确保 lower/upper bounds 与 children 数量一致
    if (lowerBounds_.size() != numChildren || upperBounds_.size() != numChildren) {
        for (size_t ci = 0; ci < numChildren; ++ci) {
            auto childResult = children_[ci]->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
        return result;
    }

    // Step 2: 对每个子节点进行剪枝判断
    for (size_t ci = 0; ci < numChildren; ++ci) {
        bool canPrune = false;

        // --- 剪枝 1: 投影交集检查 ---
        bool hasIntersection = true;
        for (size_t di = 0; di < numDimensions; ++di) {
            if (di >= lowerBounds_[ci].size() || di >= upperBounds_[ci].size()) {
                hasIntersection = false;
                break;
            }

            double proj_q = 0.0;
            for (size_t i = 0; i < pivots_.size(); ++i) {
                proj_q += distQToPivots[i] * normalVectors_[di][i];
            }

            double norm_len = 0.0;
            for (double x : normalVectors_[di]) norm_len += x * x;
            norm_len = std::sqrt(norm_len);
            double proj_radius = r * norm_len;

            double query_min = proj_q - proj_radius;
            double query_max = proj_q + proj_radius;

            double child_min = lowerBounds_[ci][di];
            double child_max = upperBounds_[ci][di];

            // 检查区间是否相交
            if (query_max < child_min || query_min > child_max) {
                hasIntersection = false;
                break;
            }
        }

        if (!hasIntersection) {
            canPrune = true;
        }

        // --- 移除 canInclude 逻辑，只做剪枝判断 ---
        if (canPrune) {
            continue; // 剪枝：跳过这个子树
        }
        else {
            // 不剪枝：递归搜索子节点
            auto childResult = children_[ci]->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
    }

    return result;
}


DataList CLPTInternalNode::getAll() const {
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