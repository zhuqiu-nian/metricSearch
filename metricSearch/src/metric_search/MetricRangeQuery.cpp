#include "../../include/metric_search/MetricRangeQuery.h"
#include "../../include/utils/Solution.h"
#include <chrono>
#include <cmath>
#include <iostream>


using namespace std;
using namespace std::chrono;

namespace MetricSpaceExtensions {

    MetricSpaceSearch::SearchResult rangeQuery(
        const vector<shared_ptr<MetricData>>& dataset,
        const shared_ptr<MetricData>& query,
        const shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const shared_ptr<MetricData>& pivot) {

        MetricSpaceSearch::SearchResult result;
        result.distanceFunc = distanceFunc;
        result.pivot = pivot ? pivot : (dataset.empty() ? nullptr : dataset[0]);
        result.calculations = 0;
        result.timeMicrosec = 0;

        if (dataset.empty() || !result.pivot) {
            auto end = high_resolution_clock::now();
            result.timeMicrosec = duration_cast<microseconds>(high_resolution_clock::now() - high_resolution_clock::now()).count();
            return result;
        }

        auto start = high_resolution_clock::now();

        // ��ȡ֧�ŵ㵽��ѯ��ľ���
        long double d_pq = distanceFunc->distance(*result.pivot, *query);
        result.calculations++;

        for (const auto& data : dataset) {
            if (data == query) continue;

            // ��ȡ֧�ŵ㵽 data �ľ���
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
                }
            }
            else {
                d_pd = distanceFunc->distance(*result.pivot, *data);
                result.calculations++;
            }

            // ���ǲ���ʽ��֦
            long double lower_bound = abs(d_pq - d_pd);
            long double upper_bound = d_pq + d_pd;

            if (lower_bound > radius) {
                result.steps.push_back("���� " + data->toString() + "���½��֦��");
                continue;
            }

            if (upper_bound <= radius) {
                result.rangeResults.emplace_back(data, d_pq - d_pd); // ���Ż�Ϊʵ��ֵ��
                result.steps.push_back("���� " + data->toString() + "���Ͻ��֦��");
                continue;
            }

            // ��Ҫʵ�ʼ��� query �� data �ľ���
            long double actual_dist = distanceFunc->distance(*query, *data);
            result.calculations++;
            if (actual_dist <= radius) {
                result.rangeResults.emplace_back(data, actual_dist);
                result.steps.push_back("���� " + data->toString() + "����ʵ���� �� �뾶��");
            }
            else {
                result.steps.push_back("�ų� " + data->toString() + "����ʵ���� > �뾶��");
            }
        }

        auto end = high_resolution_clock::now();
        result.timeMicrosec = duration_cast<microseconds>(end - start).count();

        return result;
    }

    void runRangeQuery(
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

        // ���� rangeQuery ��������
        cout << "\n��ʼ��Χ����...\n";

        MetricSpaceSearch::SearchResult result = MetricSpaceExtensions::rangeQuery(dataset, queryPtr, distanceFunc, radius, pivot);

        // ʹ��ͳһ��ӡ����������
        printSearchResult(result);
    }

} // namespace MetricSpaceExtensions
