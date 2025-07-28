// VPTree.cpp
#include "../../../include/index_structure/VantagePointTree/VPTree.h"
#include "../../../include/index_structure/VantagePointTree/VPTInternalNode.h"
#include "../../../include/index_structure/VantagePointTree/VPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
<<<<<<< HEAD
#include "../../../include/utils/Solution.h"
=======
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d

#include <iostream>
#include <algorithm>

const int MaxLeafSize = 20;

long long VPTree::distanceCalculations_ = 0;

std::unique_ptr<VPTNode> VPTree::bulkLoad(const std::vector<DataPtr>& data,
    int distanceType,
    int dataType) {
    if (data.size() <= MaxLeafSize) {
        return std::make_unique<VPTLeafNode>(data, distanceType, dataType);
    }

    auto [vp, radius] = selectVPAndRadius(data, distanceType, dataType);

    DataList leftData, rightData;
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    for (const auto& item : data) {
        if (item == vp) continue;
        long double d = dist->distance(*item, *vp);
        if (d <= radius)
            leftData.push_back(item);
        else
            rightData.push_back(item);
    }

    auto left = bulkLoad(leftData, distanceType, dataType);
    auto right = bulkLoad(rightData, distanceType, dataType);

    return std::make_unique<VPTInternalNode>(vp, radius, std::move(left), std::move(right), dist);
}

//ѡȡ֧�ŵ�ͻ��ְ뾶��Ĭ��ѡ���һ������Ϊ֧�ŵ�
std::pair<std::shared_ptr<MetricData>, long double> VPTree::selectVPAndRadius(
    const std::vector<std::shared_ptr<MetricData>>& data, int distanceType, int dataType) {

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // ��ѡȡ��һ������Ϊ֧�ŵ㣨�����Ż�Ϊ��󷽲�ȣ�
    auto vp = data[0];

    long double medianRadius = getMedianRadius(data, *vp, *dist);

    return { vp, medianRadius };
}

//ȡ��λ����Ϊ�뾶
long double VPTree::getMedianRadius(const std::vector<std::shared_ptr<MetricData>>& data,
    const MetricData& vp,
    const MetricDistance& dist) {
    std::vector<long double> distances;
    for (const auto& item : data) {
        if (item.get() != &vp) {
            distances.push_back(dist.distance(vp, *item));
        }
    }

    if (distances.empty()) return 0.0L;

    std::sort(distances.begin(), distances.end());
    return distances[distances.size() / 2];
}

void VPTree::runVPTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "���ݼ�Ϊ�գ��޷�ִ�в�ѯ��" << std::endl;
        return;
    }

    // ���� VP ��
    auto treeRoot = VPTree::bulkLoad(dataset, distanceType, dataType);

<<<<<<< HEAD
    // �û�ѡ���ѯ����Դ
    int querySource;
    cout << "��ѡ���ѯ����Դ��\n"
        << "1 - ���������ݼ���ѡ��\n"
        << "2 - �Զ��������²�ѯ��\n"
        << "������ѡ���ţ�";
    cin >> querySource;

    if (cin.fail() || (querySource != 1 && querySource != 2)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "������Ч����ѡ�� 1 �� 2��\n";
        return;
    }

    shared_ptr<MetricData> queryPtr;

    if (querySource == 1) {
        int queryIndex;
        cout << "��ѡ��һ����ѯ����������0 �� " << dataset.size() - 1 << "����";
        cin >> queryIndex;

        if (cin.fail() || queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "������Ч����������Ч��������Χ��\n";
            return;
        }
        queryPtr = dataset[queryIndex];
    }
    else {
        try {
            queryPtr = inputCustomQuery(dataType);
        }
        catch (const exception& e) {
            cout << "�Զ�������ʧ�ܣ�" << e.what() << endl;
            return;
        }
    }


=======
    // ��ȡ�û����룺��ѯ��������
    int queryIndex;
    std::cout << "��ѡ���ѯ�������� (0-" << dataset.size() - 1 << "): ";
    std::cin >> queryIndex;

    if (queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
        std::cerr << "��Ч�Ĳ�ѯ����������" << std::endl;
        return;
    }

>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    // ��ȡ�û����룺��ѯ�뾶 r
    long double threshold;
    std::cout << "�������ѯ�뾶 r: ";
    std::cin >> threshold;

    if (threshold < 0) {
        std::cerr << "��ѯ�뾶����Ϊ������" << std::endl;
        return;
    }

    // ��ȡ��ѯ����
<<<<<<< HEAD
    const MetricData& query = *queryPtr;
=======
    const MetricData& query = *dataset[queryIndex];
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d

    // ���ü�����
    VPTree::resetDistanceCalculations();

    // ִ�з�Χ��ѯ
    auto results = treeRoot->rangeSearch(query, threshold, &VPTree::distanceCalculations_);

    // ���˵���ѯ������
<<<<<<< HEAD
=======
    auto queryPtr = dataset[queryIndex];
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // �����ǰ��ѯ������Ϣ
<<<<<<< HEAD
    std::cout << "\n- ��ѯ���� #" << queryPtr << ": " << queryPtr->toString() << std::endl;
=======
    std::cout << "\n- ��ѯ���� #" << queryIndex << ": " << dataset[queryIndex]->toString() << std::endl;
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d

    // ���ƥ��������
    std::cout << "\n�ҵ�ƥ������������������ѯ��������: " << filteredResults.size() << std::endl;

    if (!filteredResults.empty()) {
        std::cout << "������ƥ���" << std::endl;
        for (size_t i = 0; i < filteredResults.size(); ++i) {
            std::cout << "  - ƥ���� #" << i + 1 << ": " << filteredResults[i]->toString() << std::endl;
        }
    }
    else {
        std::cout << "δ�ҵ��κ�ƥ���" << std::endl;
    }

    // �������������
    std::cout << "\n���β�ѯ�����þ��뺯��: " << VPTree::getDistanceCalculations() << " ��" << std::endl;
}

