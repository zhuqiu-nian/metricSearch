// MVPTLeafNode.cpp

#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"

MVPTLeafNode::MVPTLeafNode(const DataList& data, int distanceType, int dataType)
<<<<<<< HEAD
    : dataList_(data), distanceType_(distanceType), dataType_(dataType), isEmpty_(true)
{
    // 先判断数据是否为空
    if (data.empty()) {
        // 空节点：不构造 PivotTable，保持 isEmpty_ = true
        return;
    }

    // 数据非空，再尝试构造 PivotTable
    try {
        pivotTable_ = std::make_unique<PivotTable>(data, 1, distanceType, dataType);  // 正常构造
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        // 如果构造失败（比如 k=1 但数据有问题），也视为空节点
        std::cerr << "[MVPTLeafNode] 构造 PivotTable 失败: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> MVPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    if (!pivotTable_) {  // 或 if (isEmpty_)
        return {};
    }
    return pivotTable_->search(q, r, distanceCount);
=======
    : pivotTable_(data,1, distanceType, dataType), dataList_(data) {}

std::vector<DataPtr> MVPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    return pivotTable_.search(q, r, distanceCount);
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
}

DataList MVPTLeafNode::getAll() const {
    return dataList_;
}