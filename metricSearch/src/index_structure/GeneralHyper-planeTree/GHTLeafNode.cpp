#include "../../../include/index_structure/GeneralHyper-planeTree/GHTLeafNode.h"
#include "../../../include/utils/MetricSpaceSearch.h"

GHTLeafNode::GHTLeafNode(const DataList& data, int distanceType, int dataType)
    : pivotTable_(data, 4, distanceType, dataType) {}  // Ĭ��ѡ4��֧�ŵ�

std::vector<DataPtr> GHTLeafNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    return pivotTable_.search(q, r, distanceCount);  // �Ѿ�����˵���ѯ������
}