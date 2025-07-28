// MVPTLeafNode.cpp

#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"

MVPTLeafNode::MVPTLeafNode(const DataList& data, int distanceType, int dataType)
<<<<<<< HEAD
    : dataList_(data), distanceType_(distanceType), dataType_(dataType), isEmpty_(true)
{
    // ���ж������Ƿ�Ϊ��
    if (data.empty()) {
        // �սڵ㣺������ PivotTable������ isEmpty_ = true
        return;
    }

    // ���ݷǿգ��ٳ��Թ��� PivotTable
    try {
        pivotTable_ = std::make_unique<PivotTable>(data, 1, distanceType, dataType);  // ��������
        isEmpty_ = false;
    }
    catch (const std::exception& e) {
        // �������ʧ�ܣ����� k=1 �����������⣩��Ҳ��Ϊ�սڵ�
        std::cerr << "[MVPTLeafNode] ���� PivotTable ʧ��: " << e.what() << std::endl;
        isEmpty_ = true;
    }
}

std::vector<DataPtr> MVPTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    if (!pivotTable_) {  // �� if (isEmpty_)
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