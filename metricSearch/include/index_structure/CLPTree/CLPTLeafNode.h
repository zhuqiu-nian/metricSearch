// CLPTLeafNode.h
#ifndef CLPTLEAFNODE_H
#define CLPTLEAFNODE_H

#include "../../../include/index_structure/CLPTree/CLPTNode.h"
#include "../../../include/index_structure/PivotTable/PivotTable.h"          // ← 新增
#include "../../../src/PivotSelector/PivotSelector.h"   // ← 新增
#include <memory>
#include <vector>

using DataPtr = std::shared_ptr<MetricData>;
using DataList = std::vector<DataPtr>;

class CLPTLeafNode : public CLPTNode {
public:
    // 构造函数：接收 distanceType, dataType, method，用于构建 PivotTable
    CLPTLeafNode(
        const DataList& data,
        int distanceType,
        int dataType,
        PivotSelector::SelectionMethod method,
        int pivotCountForLeaf = 1  // 可配置，默认 1 个支撑点
    );

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    DataList getAll() const override;

private:
    DataList dataList_;
    std::unique_ptr<PivotTable> pivotTable_;  // ← 改为持有 PivotTable
    bool isEmpty_ = false;
};

#endif // CLPTLEAFNODE_H