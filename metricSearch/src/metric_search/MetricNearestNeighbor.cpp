#include "../../include/metric_search/MetricNearestNeighbor.h"
#include "../../include/utils/Solution.h"
#include <iostream>
#include <unordered_map>
#include <limits>
#include <cmath>

using namespace std;

MetricSpaceSearch::SearchResult MetricSpaceNNQuery::nearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    const shared_ptr<MetricData>& pivot)
{
    const long double EPS = 1e-15;
    MetricSpaceSearch::SearchResult result;
    result.pivot = pivot;
    result.distanceFunc = distanceFunc;
    result.calculations = 0;

    if (dataset.empty()) {
        return result; // ���ݼ�Ϊ�գ�ֱ�ӷ��ؿս��
    }

    // ���δָ��֧�ŵ㣬��Ĭ��ʹ�õ�һ�����ݵ���Ϊ֧�ŵ�
    shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // d(p, q)��֧�ŵ㵽��ѯ��ľ��루���ȴӻ����л�ȡ��
    long double d_pq;
    if (MetricSpaceSearch::pivotDistanceCache.count(initialPivot.get()) &&
        MetricSpaceSearch::pivotDistanceCache[initialPivot.get()].count(query.get()))
    {
        d_pq = MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else
    {
        cout << "\n----------------[DEBUG]:Ԥ������ȫ----------------\n";
        d_pq = distanceFunc->distance(*initialPivot, *query);
        MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][query.get()] = d_pq;
    }

    // ��ʼ�������Ϊ�����Ϳ�ָ��
    result.distance = numeric_limits<long double>::max();
    result.nearest.reset();

    // === ֱ��ѡ���һ����֧�ŵ㡢�ǲ�ѯ����Ϊ��ʼ����� ===
    bool foundInitial = false;
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query) {
            continue; // ����֧�ŵ�Ͳ�ѯ��
        }

        // �ҵ���һ�����������ĵ�
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;
        result.distance = d_qx;
        result.nearest = data;
        result.steps.push_back("��ʼ�������Ϊ: " + data->toString() +
            "����ʼ����Ϊ: " + to_string(d_qx));
        foundInitial = true;
        break;
    }

    if (!foundInitial) {
        result.steps.push_back("���棺û���ҵ����õĳ�ʼ����ڣ�");
        return result;
    }

    // === ��ʼ���������ݼ��������� ===
    for (const auto& data : dataset)
    {
        if (data == initialPivot || data == query || data == result.nearest) {
            continue; // ����֧�ŵ㡢��ѯ��͵�ǰ�����
        }

        // ��ȡ d(p, x)����֧�ŵ㵽��ǰ���ݵ�ľ���
        long double d_px;
        if (MetricSpaceSearch::pivotDistanceCache.count(initialPivot.get()) &&
            MetricSpaceSearch::pivotDistanceCache[initialPivot.get()].count(data.get()))
        {
            d_px = MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][data.get()];
        }
        else
        {
            d_px = distanceFunc->distance(*initialPivot, *data);
            MetricSpaceSearch::pivotDistanceCache[initialPivot.get()][data.get()] = d_px;
        }

        // ====== ʹ�����ǲ���ʽ���м�֦ ======
        long double lower_bound = abs(d_pq - d_px);   // �½�
        long double upper_bound = d_pq + d_px;        // �Ͻ�

        // Debug �����֦��Ϣ
        result.steps.push_back("[DEBUG] ��ǰ��: " + data->toString() +
            ", d_px=" + to_string(d_px) +
            ", lower_bound=" + to_string(lower_bound) +
            ", upper_bound=" + to_string(upper_bound) +
            ", current_min=" + to_string(result.distance));

        // �Ͻ��֦������Ͻ�С�ڵ�ǰ��С���룬����
        if (upper_bound < result.distance - EPS) {
            result.steps.push_back("�����Ͻ��֦����: " + data->toString());
            continue;
        }

        // �½��֦������½���ڵ��ڵ�ǰ��С���룬����
        if (lower_bound >= result.distance + EPS) {
            result.steps.push_back("�����½��֦����: " + data->toString());
            continue;
        }

        // �޷���֦�����������ʵ����
        long double d_qx = distanceFunc->distance(*query, *data);
        result.calculations++;

        result.steps.push_back("�����ѯ�㵽 " + data->toString() +
            " ��ʵ�ʾ���: " + to_string(d_qx));

        // ���������
        if (d_qx < result.distance) {
            result.distance = d_qx;
            result.nearest = data;
            result.steps.push_back("�����µ������: " + data->toString() +
                " �������Ϊ " + to_string(d_qx));
        }
    }

    //����ж�֧�ŵ��ǲ�������ڣ���Ϊ֮ǰ������֧�ŵ�
    if (d_pq < result.distance) {
        result.distance = d_pq;
        result.nearest = initialPivot;
        result.steps.push_back("�����µ������: ��֧�ŵ㣡�������Ϊ " + to_string(d_pq));
    }

    return result;
}


void runNearestNeighborQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        cout << "�������ݼ�Ϊ�ա�\n";
        return;
    }

    char showDataChoice;
    cout << "�Ƿ�չʾ�Ѷ�������ݼ���(y/n): ";
    cin >> showDataChoice;

    if (showDataChoice == 'y' || showDataChoice == 'Y') {
        cout << "\n--- �Ѷ�������ݼ� ---\n";
        for (const auto& item : dataset) {
            cout << item->toString() << endl;
        }
        cout << "------------------------\n\n";
    }

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

    // �û�ѡ��֧�ŵ�
    int pivotIndex;
    cout << "��ѡ��һ��֧�ŵ�������0 �� " << dataset.size() - 1 << "����";
    cin >> pivotIndex;

    if (cin.fail() || pivotIndex < 0 || pivotIndex >= static_cast<int>(dataset.size())) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "������Ч����������Ч��������Χ��\n";
        return;
    }

    auto pivot = dataset[pivotIndex];

    // ������Ӧ�ľ��뺯��
    shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // ���� nearestNeighbor ��������
    cout << "\n��ʼ���������...\n";

    MetricSpaceSearch::SearchResult result = MetricSpaceNNQuery::nearestNeighbor(dataset, queryPtr, distanceFunc, pivot);

    // ʹ��ͳһ��ӡ����������
    printSearchResult(result);
}