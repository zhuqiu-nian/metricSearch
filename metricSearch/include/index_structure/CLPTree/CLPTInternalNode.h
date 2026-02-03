#ifndef CLPTINTERNALNODE_H
#define CLPTINTERNALNODE_H

#include "CLPTNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include <vector>
#include <memory>

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

class CLPTInternalNode : public CLPTNode {
public:
    /**
     * @brief 构造内部节点
     * @param pivots 当前节点的支撑点列表
     * @param children 子节点列表
     * @param normalVectors 当前节点使用的法向量组
     * @param lowerBounds 每个子节点在每个支撑点/法向量维度上的距离下界
     * @param upperBounds 每个子节点在每个支撑点/法向量维度上的距离上界
     * @param longestDistancesToPivots 每个子节点到每个支撑点的最远距离
     * @param dist 距离函数
     */
    CLPTInternalNode(
        std::vector<DataPtr> pivots,
        std::vector<std::unique_ptr<CLPTNode>> children,
        std::vector<std::vector<double>> normalVectors,
        std::vector<std::vector<double>> lowerBounds,
        std::vector<std::vector<double>> upperBounds,
        std::vector<std::vector<double>> longestDistancesToPivots,
        std::shared_ptr<MetricDistance> dist
    );

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    std::vector<DataPtr> pivots_;
    std::vector<std::unique_ptr<CLPTNode>> children_;
    std::vector<std::vector<double>> normalVectors_; // 核心：完全线性划分使用的法向量组
    std::vector<std::vector<double>> lowerBounds_;
    std::vector<std::vector<double>> upperBounds_;
    std::vector<std::vector<double>> longestDistancesToPivots_; // 用于包含关系剪枝
    std::shared_ptr<MetricDistance> dist_;
};

#endif // CLPTINTERNALNODE_H