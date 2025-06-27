#include "../../include/metric_search/MetricSpaceBoundedKNN.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include "../../include/utils/Solution.h"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_set>
#include <chrono>

using namespace std;
using namespace std::chrono;

// �Զ���Ƚ���������ά�����ѣ������뽵������
struct CompareDistance {
    bool operator()(const pair<shared_ptr<MetricData>, long double>& a,
        const pair<shared_ptr<MetricData>, long double>& b) {
        return a.second < b.second; // ����
    }
};

MetricSpaceSearch::SearchResult boundedKnnQuery(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    int k,
    const shared_ptr<MetricData>& pivot,
    long double radius)
{
    MetricSpaceSearch::SearchResult result;
    result.distanceFunc = distanceFunc;
    result.k = k;
    result.calculations = 0;
    result.timeMicrosec = 0;

    if (dataset.empty() || k <= 0) {
        auto end = high_resolution_clock::now();
        result.timeMicrosec = duration_cast<microseconds>(end - high_resolution_clock::now()).count();
        return result;
    }

    auto start = high_resolution_clock::now();

    // ����֧�ŵ�
    result.pivot = pivot ? pivot : dataset[0];
    if (!result.pivot) {
        return result;
    }

    // ��ȡ֧�ŵ㵽��ѯ��ľ���
    long double d_pq = distanceFunc->distance(*result.pivot, *query);
    result.calculations++;

    // ʹ�����ѱ��浱ǰ��Զ�� k ���ھӣ��Ҿ��� �� radius��
    priority_queue<pair<shared_ptr<MetricData>, long double>,
        vector<pair<shared_ptr<MetricData>, long double>>,
        CompareDistance> maxHeap;

    for (const auto& data : dataset) {
        if (data == query) continue;

        // ��ȡ֧�ŵ㵽 data �ľ��루����ʹ�û��棩
        long double d_pd = 0.0L;
        auto it_pivot_data = MetricSpaceSearch::pivotDistanceCache.find(result.pivot.get());
        if (it_pivot_data != MetricSpaceSearch::pivotDistanceCache.end()) {
            auto it_data = it_pivot_data->second.find(data.get());
            if (it_data != it_pivot_data->second.end()) {
                d_pd = it_data->second; // ��������
            }
            else {
                d_pd = distanceFunc->distance(*result.pivot, *data);
                result.calculations++;
                MetricSpaceSearch::pivotDistanceCache[result.pivot.get()][data.get()] = d_pd;
            }
        }
        else {
            d_pd = distanceFunc->distance(*result.pivot, *data);
            result.calculations++;
            MetricSpaceSearch::pivotDistanceCache[result.pivot.get()][data.get()] = d_pd;
        }

        // ���ǲ���ʽ��֦
        long double lower_bound = abs(d_pq - d_pd);
        long double upper_bound = d_pq + d_pd;

        // ����½� > radius���򲻿�����������
        if (lower_bound > radius) {
            result.steps.push_back("���� " + data->toString() + "���½��֦��������Χ��");
            continue;
        }

        // ����Ͻ� �� radius�������ֱ�Ӽ��룬����Ҫʵ�ʾ�������ж��Ƿ���� kNN �б�
        long double actual_dist;
        if (upper_bound <= radius) {
            result.steps.push_back("���� " + data->toString() + "���Ͻ��֦��");
            actual_dist = d_pq + d_pd; // ���Ż�Ϊ��ʵ�������
        }
        else {
            // ����������ʵ�ʾ���
            actual_dist = distanceFunc->distance(*query, *data);
            result.calculations++;
        }

        // �ж��Ƿ��ڷ�Χ��
        if (actual_dist > radius) {
            result.steps.push_back("�ų� " + data->toString() + "�������뾶��");
            continue;
        }

        // �������� kNN ��ѡ�б�
        if (maxHeap.size() < static_cast<size_t>(k)) {
            maxHeap.push({ data, actual_dist });
            result.steps.push_back("���� " + data->toString() + "����ѡ����ڣ�");
        }
        else if (actual_dist < maxHeap.top().second) {
            maxHeap.pop();
            maxHeap.push({ data, actual_dist });
            result.steps.push_back("���������: " + data->toString());
        }
        else {
            result.steps.push_back("�ų� " + data->toString() + "��������ڣ�");
        }

        // ��������ں���С����
        if (actual_dist < result.distance) {
            result.distance = actual_dist;
            result.nearest = data;
        }
    }

    // �������е�Ԫ��ȡ��������������
    while (!maxHeap.empty()) {
        result.knnResults.push_back(maxHeap.top());
        maxHeap.pop();
    }
    reverse(result.knnResults.begin(), result.knnResults.end());

    auto end = high_resolution_clock::now();
    result.timeMicrosec = duration_cast<microseconds>(end - start).count();

    return result;
}

void runBoundedKnnQuery(
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

    // ���� k ֵ
    int k;
    cout << "������Ҫ���ҵ�����ڸ��� k��";
    cin >> k;

    if (cin.fail() || k <= 0 || k > static_cast<int>(dataset.size())) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "������Ч��������һ������ 0 ��С�ڵ������ݼ���С��ֵ��\n";
        return;
    }

    // �������������� radius
    long double radius;
    cout << "��������������뾶��radius����";
    cin >> radius;

    if (cin.fail() || radius < 0.0L) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "������Ч��������һ���Ǹ���ֵ��\n";
        return;
    }

    // ������Ӧ�ľ��뺯��
    shared_ptr<MetricDistance> distanceFunc = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    // ���� boundedKnnQuery ��������
    cout << "\n��ʼ���������Ƶ� kNN ����...\n";

    MetricSpaceSearch::SearchResult result = boundedKnnQuery(dataset, queryPtr, distanceFunc, k, pivot, radius);

    // ʹ��ͳһ��ӡ����������
    printSearchResult(result);
}