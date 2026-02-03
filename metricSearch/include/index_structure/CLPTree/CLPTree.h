#ifndef CLPTREE_H
#define CLPTREE_H

#include "../../../include/interfaces/MetricData.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/index_structure/CLPTree/CLPTNode.h"
#include "../src/PivotSelector/PivotSelector.h"
#include <vector>
#include <memory>
#include <iostream>

// Forward declarations
class CLPTNode;
class CLPTInternalNode;
class CLPTLeafNode;

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

class CLPTree {
public:
    static std::unique_ptr<CLPTNode> bulkLoadRecursive(
        const DataList& data,
        const std::vector<DataPtr>& globalPivots,
        const std::vector<std::vector<double>>& normalVectors,
        int currentNormalIndex,
        int f,
        int distanceType,
        int dataType,
        PivotSelector::SelectionMethod method,
        MetricDistance* dist,
        long long* buildDistCount
    );

    /**
     * @brief 从数据集批量构建 CLP 树
     * @param data 数据集
     * @param k 每层使用的支撑点数量
     * @param f 每个支撑点划分的扇出数
     * @param distanceType 距离函数类型
     * @param dataType 数据类型
     * @param maxLeafSize 叶节点最大容量
     * @return 构建好的树的根节点
     */
    static std::unique_ptr<CLPTNode> bulkLoad(
        const DataList& data,
        int k,
        int f,
        int distanceType,
        int dataType,
        PivotSelector::SelectionMethod method
    );

    /**
     * @brief 运行范围查询的主入口
     * @param dataset 数据集
     * @param distanceType 距离函数类型
     * @param dataType 数据类型
     */
    static void runCLPTRangeSearch(const DataList& dataset, int distanceType, int dataType);

    // 获取查询阶段距离计算次数
    static long long getQueryDistanceCalculations() { return queryDistanceCalculations_; }
    static void resetDistanceCalculations() { queryDistanceCalculations_ = 0; }
    // （可选）获取构建阶段距离计算次数
    static long long getBuildDistanceCalculations() { return buildDistanceCalculations_; }


private:
    static long long buildDistanceCalculations_;
    static long long queryDistanceCalculations_;
};

#endif // CLPTREE_H