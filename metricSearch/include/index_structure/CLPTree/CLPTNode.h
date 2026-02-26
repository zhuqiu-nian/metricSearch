#ifndef CLPTNODE_H
#define CLPTNODE_H

#include "../../../include/interfaces/MetricData.h"
#include "../../../include/interfaces/MetricDistance.h"
#include <vector>
#include <memory>

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

/**
 * @brief CLP 树节点基类
 */
class CLPTNode {
public:
    virtual ~CLPTNode() = default;

    /**
     * @brief 在当前节点及其子树中执行范围查询
     * @param q 查询点
     * @param r 查询半径
     * @param distanceCount 距离计算次数计数器
     * @return 符合条件的数据点列表
     */
    virtual std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) = 0;

    /**
     * @brief 获取当前节点及其子树中的所有数据点
     * @return 所有数据点列表
     */
    virtual DataList getAll() const = 0;
};

#endif // CLPTNODE_H