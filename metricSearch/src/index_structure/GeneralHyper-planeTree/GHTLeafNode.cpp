#include "../../../include/index_structure/GeneralHyper-planeTree/GHTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include <random>

GHTLeafNode::GHTLeafNode(const DataList& data, int distanceType, int dataType, PivotSelector::SelectionMethod method, int pivotCountForLeaf)//默认使用一个支撑点
    : dataList_(data), distanceType_(distanceType), dataType_(dataType), isEmpty_(true)
{
    // 先判断数据是否为空
    if (data.empty()) {
        // 空节点：不构造 PivotTable，保持 isEmpty_ = true
        return;
    }

    // 数据非空，再尝试构造 PivotTable
    try {
        // 随机选 1 个索引
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, static_cast<int>(data.size()) - 1);
        int pivotIndex = dis(gen);

        std::vector<int> selectedPivots = { pivotIndex };

        // 构造 PivotTable（假设 PivotTable 接受索引列表）
        pivotTable_ = std::make_unique<PivotTable>(data, selectedPivots, distanceType, dataType);
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        // 如果构造失败（比如 k=1 但数据有问题），也视为空节点
        std::cerr << "[MVPTLeafNode] 构造 PivotTable 失败: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> GHTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    if (!pivotTable_) {  // 或 if (isEmpty_)
        return {};
    }
    return pivotTable_->search(q, r, distanceCount);  // 已经会过滤掉查询对象本身
}