// CGHTInternalNode.cpp
#include "../../../include/index_structure/CGHTree/CGHTInternalNode.h"
#include "../../../include/index_structure/CGHTree/CGHTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include <algorithm>

CGHTInternalNode::CGHTInternalNode(
    DataPtr pivot1, DataPtr pivot2, DataPtr pivot3,
    std::vector<std::unique_ptr<CGHTNode>> children,
    std::vector<std::pair<long double, long double>> feature1Ranges,
    std::vector<std::pair<long double, long double>> feature2Ranges,
    std::vector<std::pair<long double, long double>> feature3Ranges,
    std::shared_ptr<MetricDistance> dist)
    : pivot1_(pivot1), pivot2_(pivot2), pivot3_(pivot3),
    children_(std::move(children)),
    feature1Ranges_(std::move(feature1Ranges)),
    feature2Ranges_(std::move(feature2Ranges)),
    feature3Ranges_(std::move(feature3Ranges)),
    dist_(std::move(dist)) {}

std::vector<DataPtr> CGHTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    // 计算查询点到三个pivots的距离
    long double d1_query = dist_->distance(q, *pivot1_);
    long double d2_query = dist_->distance(q, *pivot2_);
    long double d3_query = dist_->distance(q, *pivot3_);
    if (distanceCount) (*distanceCount) += 3; // 计算三次距离

    // 计算查询点的三个特征值
    long double f1_query = d1_query + d2_query - d3_query;  // d1+d2-d3
    long double f2_query = d1_query - d2_query + d3_query;  // d1-d2+d3
    long double f3_query = d1_query - d2_query - d3_query;  // d1-d2-d3

    // 检查pivots本身是否满足查询条件
    if (d1_query <= r) {
        result.push_back(pivot1_);
    }
    if (d2_query <= r && pivot1_ != pivot2_) {
        result.push_back(pivot2_);
    }
    if (d3_query <= r && pivot1_ != pivot3_ && pivot2_ != pivot3_) {
        result.push_back(pivot3_);
    }

    // 对每个子节点进行剪枝判断
    for (size_t ci = 0; ci < children_.size(); ++ci) {
        // 获取当前子节点的特征值范围
        long double f1_min = feature1Ranges_[ci].first;
        long double f1_max = feature1Ranges_[ci].second;
        long double f2_min = feature2Ranges_[ci].first;
        long double f2_max = feature2Ranges_[ci].second;
        long double f3_min = feature3Ranges_[ci].first;
        long double f3_max = feature3Ranges_[ci].second;

        // 三维剪枝条件：查询点的特征值是否可能落在子节点的范围内
        // 这里使用简单的边界扩展进行剪枝
        bool f1_condition = (f1_min - 3 * r <= f1_query) && (f1_query <= f1_max + 3 * r);
        bool f2_condition = (f2_min - 3 * r <= f2_query) && (f2_query <= f2_max + 3 * r);
        bool f3_condition = (f3_min - 3 * r <= f3_query) && (f3_query <= f3_max + 3 * r);

        if (f1_condition && f2_condition && f3_condition) {
            // 不能剪枝，递归搜索子节点
            auto childResult = children_[ci]->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
        // 如果任一条件不满足，则该子节点被剪枝，无需进一步搜索
    }

    return result;
}

std::vector<DataPtr> CGHTInternalNode::getAll() const {
    std::vector<DataPtr> all;
    all.push_back(pivot1_);
    all.push_back(pivot2_);
    all.push_back(pivot3_);
    for (const auto& child : children_) {
        auto childAll = child->getAll();
        all.insert(all.end(), childAll.begin(), childAll.end());
    }
    return all;
}