#include "../../../include/index_structure/GeneralHyper-planeTree/GHTree.h"
#include "../../../include/utils/MetricSpaceSearch.h"  // ���� createDistanceFunction
#include "../../../include/index_structure/GeneralHyper-planeTree/GHTInternalNode.h"
#include "../../../include/index_structure/GeneralHyper-planeTree/GHTLeafNode.h"
#include "../../../include/utils/Solution.h"
#include "../../PivotSelector/PivotSelector.h"
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>

long long GHTree::distanceCalculations_ = 0;

// ���ѡ������֧�ŵ�
std::pair<DataPtr, DataPtr> GHTree::selectPivots(const DataList& data) {
    std::vector<int> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    return { data[indices[0]], data[indices[1]] };
}

// �������� GHT ��
std::unique_ptr<GHTNode> GHTree::bulkLoad(const DataList& data,
    int distanceType,
    int dataType,
    std::vector<int> selectedPivots)
{
    const int MaxLeafSize = 20;

    if (data.size() <= MaxLeafSize) {
        return std::make_unique<GHTLeafNode>(data, distanceType, dataType, selectedPivots);
    }

    auto [c1, c2] = selectPivots(data);

    // �������뺯��
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    DataList leftData, rightData;
    for (const auto& item : data) {
        if (item == c1 || item == c2) continue;

        long double d1 = dist->distance(*item, *c1);
        long double d2 = dist->distance(*item, *c2);

        if (d1 <= d2) {
            leftData.push_back(item);
        }
        else {
            rightData.push_back(item);
        }
    }

    auto left = bulkLoad(leftData, distanceType, dataType, selectedPivots);
    auto right = bulkLoad(rightData, distanceType, dataType, selectedPivots);

    return std::make_unique<GHTInternalNode>(c1, c2, std::move(left), std::move(right), dist);
}

void GHTree::runGHTRangeSearch(const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "���ݼ�Ϊ�գ��޷�ִ�в�ѯ��" << std::endl;
        return;
    }

    //����֧�ŵ����
    int pivotCount;
    std::cout << "������֧�ŵ����: ";
    std::cin >> pivotCount;

    if (pivotCount <= 0 || pivotCount >= static_cast<int>(dataset.size())) {
        std::cerr << "֧�ŵ�����������0��С������������" << std::endl;
        return;
    }

    //ѡ��֧�ŵ�ѡ���㷨
    PivotSelector::SelectionMethod method = PivotSelector::selectPivotMethodFromUser();

    //�������뺯��
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);  // �������뺯��

    //���� PivotSelector ����֧�ŵ�����
    double alpha = 0.35; // ϡ��ռ䷨����
    std::vector<int> selectedPivots = PivotSelector::selectPivots(
        dataset,
        pivotCount,
        dist,
        method,
        alpha
    );

    // ���� GHT ��
    auto treeRoot = GHTree::bulkLoad(dataset, distanceType, dataType, selectedPivots);

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


    // ��ȡ�û����룺��ѯ�뾶 r
    long double threshold;
    std::cout << "�������ѯ�뾶 r: ";
    std::cin >> threshold;

    if (threshold < 0) {
        std::cerr << "��ѯ�뾶����Ϊ������" << std::endl;
        return;
    }

    // ��ȡ��ѯ����
    const MetricData& query = *queryPtr;

    // ִ�з�Χ��ѯ
    auto results = treeRoot->rangeSearch(query, threshold, &GHTree::distanceCalculations_);

    // ���˵���ѯ������
    //ע�⣬�˴���query������Pivottable�����������Դ˴�д����ͬ
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != queryPtr.get()) {
            filteredResults.push_back(item);
        }
    }

    // �����ǰ��ѯ������Ϣ
    std::cout << "\n- ��ѯ���� #" << queryPtr << ": " << queryPtr->toString() << std::endl;

    // ������
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
    std::cout << "\n���β�ѯ�����þ��뺯��: " << GHTree::getDistanceCalculations() << " ��" << std::endl;
}