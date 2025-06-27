#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/Solution.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include "../../include/utils/LoadData.h"
#include "../../include/metric_search/all_search.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <stdexcept>

using namespace std;
using namespace std::chrono;
using DataList = std::vector<std::shared_ptr<MetricData>>;




// ʵ�����к�������ѯģʽ��չʾ���ܣ�����������ĵ�һ�룬�ҷ��������datasetֻʹ����һ�־��뺯�������Բ����ܽ��ж���뺯����ѯ����������д��һ�����ԶԶ���뺯����ѯ�ĺ���
void runExperiments(
    const vector<shared_ptr<MetricData>>& dataset,
    int data_num,
    int enableAllDistances,  // 0=���У�1/2/3=ָ������
    int data_var)            // 1=������2=�ַ���;3=������
{
    cout << "[��Ϣ] ��ʼ���ж���뺯��ʵ��..." << endl;

    if (dataset.empty()) {
        cerr << "[����] ���ݼ�Ϊ�ա�" << endl;
        return;
    }

    vector<shared_ptr<MetricDistance>> distanceFuncs;

    if (enableAllDistances == 0) {
        // ʹ�ø�������о��뺯��
        distanceFuncs = MetricSpaceSearch::getAllDistanceFunctions(data_var);
    }
    else {
        // ʹ��ָ���ĵ������뺯��
        auto func = MetricSpaceSearch::createDistanceFunction(enableAllDistances, data_var);
        distanceFuncs = { func };
    }

    for (int i = 0; i < data_num && i < dataset.size(); ++i) {
        auto query = dataset[i];
        cout << "\n\n===== ʵ��: ��ѯ����Ϊ " << query->toString() << " =====" << endl;

        vector<vector<MetricSpaceSearch::SearchResult>> allRoundResults;

        // ����ÿ��֧�ŵ�
        for (int j = 0; j < data_num && j < dataset.size(); ++j) {
            if (i == j) continue;

            auto pivot = dataset[j];
            cout << "\n--- ʹ��֧�ŵ�: " << pivot->toString() << " ---" << endl;

            vector<MetricSpaceSearch::SearchResult> roundResults;

            // ��ÿ�־��뺯��ִ��һ�β�ѯ
            for (const auto& distFunc : distanceFuncs) {
                cout << "[��Ϣ] ʹ�þ��뺯��: " << distFunc->getName() << endl;

                auto start = high_resolution_clock::now();
                auto result = MetricSpaceNNQuery::nearestNeighbor(dataset, query, distFunc, pivot);
                auto end = high_resolution_clock::now();

                result.timeMicrosec = duration_cast<microseconds>(end - start).count();
                result.distanceFunc = distFunc;
                result.pivot = pivot;

                // ������
                cout << "�����: " << result.nearest->toString() << endl;
                cout << "����: " << result.distance << endl;
                cout << "�������: " << result.calculations << endl;
                cout << "��ʱ: " << result.timeMicrosec << " ��s" << endl;

                cout << "[��������]" << endl;
                for (const auto& step : result.steps) {
                    cout << "  " << step << endl;
                }

                roundResults.push_back(result);
            }

            allRoundResults.push_back(roundResults);
        }

        // �ۺϷ���
        MetricSpaceSearch::analyzePerformance(dataset, allRoundResults, query);
    }
}

// ʵ�����к�������ѯģʽ��
void runNearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        cout << "�������ݼ�Ϊ�ա�\n";
        return;
    }

    // �Ƿ�չʾ���ݼ�
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

    // ������
    cout << "\n--- ������� ---\n";
    cout << "��ѯ��: " << queryPtr->toString() << endl;
    cout << "֧�ŵ�: " << result.pivot->toString() << endl;
    if (result.nearest) {
        cout << "����ڵ�: " << result.nearest->toString() << endl;
        cout << "���ѯ�����С����Ϊ: " << result.distance << endl;
        cout << "ʵ�ʼ������: " << result.calculations << endl;
    }
    else {
        cout << "δ�ҵ�����ڵ㡣\n";
    }

    cout << "\n--- �������̼�¼ ---\n";
    for (const auto& step : result.steps) {
        cout << step << endl;
    }
    cout << "------------------------\n";
}




// �Զ��������ѯ�㣨���� shared_ptr<MetricData>��
shared_ptr<MetricData> inputCustomQuery(int dataType) {
    if (dataType == 1) { // ��������
        int id;
        cout << "�������Զ����ѯ��� ID: ";
        cin >> id;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("ID ������Ч");
        }

        int dim;
        cout << "����������ά�ȣ�";
        cin >> dim;

        if (cin.fail() || dim <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("ά��������Ч��С�ڵ���0");
        }

        vector<long double> vec(dim);
        cout << "������ά��ֵ���ո�ָ�����";
        for (int i = 0; i < dim; ++i) {
            cin >> vec[i];
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                throw runtime_error("��ֵ������Ч");
            }
        }

        return make_shared<VectorData>(vec, id);
    }
    else if (dataType == 2) { // �ַ�������
        int id;
        cout << "�������Զ����ַ�������� ID: ";
        cin >> id;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("ID ������Ч");
        }

        string str;
        cout << "�������ַ������ݣ�";
        cin >> str;

        return make_shared<StringData>(str, id);
    }
    else {
        throw runtime_error("��֧�ֵ���������");
    }
}

//ͳһ��������߼�
void printSearchResult(const MetricSpaceSearch::SearchResult& result) {
    cout << "\n--- ������� ---\n";
    cout << "֧�ŵ�: " << result.pivot->toString() << endl;
    cout << "ʵ�ʼ������: " << result.calculations << endl;
    cout << "��ѯ��ʱ: " << result.timeMicrosec << " ΢��" << endl;

    // ��Χ��ѯ���
    if (!result.rangeResults.empty()) {
        cout << "��Χ��ѯƥ������: " << result.rangeResults.size() << endl;
        cout << "\n--- ƥ������ݵ㣨����������---\n";
        for (const auto& entry : result.rangeResults) {
            const shared_ptr<MetricData>& point = entry.first;
            long double distance = entry.second;
            cout << point->toString() << " | ����: " << distance << endl;
        }
    }

    // kNN ��ѯ���
    if (!result.knnResults.empty()) {
        cout << "kNN ��ѯ�������: " << result.knnResults.size() << " / k=" << result.k << endl;
        cout << "\n--- ����ڵ��б�����������---\n";
        for (const auto& entry : result.knnResults) {
            const shared_ptr<MetricData>& neighbor = entry.first;
            long double distance = entry.second;
            cout << neighbor->toString() << " | ����: " << distance << endl;
        }
    }

    // �������޵� kNN ��ѯ���
    if (!result.boundedKnnResults.empty()) {
        cout << "�������� kNN �������: " << result.boundedKnnResults.size()
            << " / boundedK=" << result.boundedK
            << " / radius=" << result.radius << endl;
        cout << "\n--- ����ڵ��б�����������---\n";
        for (const auto& entry : result.boundedKnnResults) {
            const shared_ptr<MetricData>& neighbor = entry.first;
            long double distance = entry.second;
            cout << neighbor->toString() << " | ����: " << distance << endl;
        }
    }

    // ��ѯ������־
    cout << "\n--- �������̼�¼ ---\n";
    for (const auto& step : result.steps) {
        cout << "- " << step << endl;
    }
    cout << "------------------------\n";
}

//������ϵͳ��Ϊ��main������࣬�Ұ����Ƕ���������
int systemLaunch() {
    int systemBegin = 1;
    while (systemBegin) {
        //ѡ��ϵͳģʽ
        int systemSet = 0;
        cout << "��ѡ��ϵͳģʽ��\n1.��ѯģʽ��չʾ��֧ͬ�ŵ㡢��ͬ���뺯���Ĳ�ѯ���ܣ�\n2.��ѯģʽ�����û�ѡ��֧�ŵ㡢��ѯ�㡢���뺯��" << endl;
        cin >> systemSet;

        //ѡ���ѯ��ʽ��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
        cout << "��ѡ�������Բ�ѯ��ʽ��1.k-����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ;4.����ڲ�ѯ\n";
        int search_var;
        cin >> search_var;

        //�������ݼ�
        int data_num = 0;  //���ݼ�����
        int dataType = 0;  //���ݼ�����
        cout << "������Ҫ��������ݼ����ͣ�1��������2.�ַ�����3.�����ʣ�" << endl;
        cin >> dataType;
        cout << "������Ҫ��ȡ�����ݸ���������С��3����" << endl;
        cin >> data_num;
        if (data_num < 3) {
            cout << "���ݼ���Ҫ����3���������ܽ���ʵ��" << endl;
            return 1;
        }

        // �������ݼ�
        auto dataset = loadUMADData(dataType, data_num);

        //ѡ����뺯��
        int distanceType = 0;
        if (dataType == 1) {
            cout << "��ѡ��ʹ�õľ��뺯��" << endl;
            cout << "1.EuclideanDistance\n2.ChebyshevDistance\n3.LonePointDistance\n4.ManhattanDistance\n��ע������0����ȫ���뺯����ѯ��";// 0=���У�1/2/3=ָ������
        }
        else if (dataType == 2) {
            cout << "��ѡ��ʹ�õľ��뺯��" << endl;
            cout << "1.EditDistance\n2.HammingDistance\n";
        }
        else if (dataType == 3) {
            cout << "��ѡ��ʹ�õľ��뺯��" << endl;
            cout << "1.WeightedEditDistance\n";
        }
        cin >> distanceType;

        //�����0������ȫ���뺯����ѯ�����򣬽��в�ѯģʽ�򵥾��뺯����ѯ
        if (distanceType == 0) {

        }
        else {

            //��ȡ���뺯��
            auto func = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

            //Ԥ���������뺯��
            MetricSpaceSearch::precomputeDistances(dataset, func);

            if (systemSet == 2) {
                switch (search_var)
                {
                case 1:
                    cout << "\n=== ִ����ͨ kNN ��ѯ ===\n";
                    runKnnQuery(dataset, distanceType, dataType);
                    break;

                case 2:
                    cout << "\n=== ִ�з�Χ��ѯ ===\n";
                    MetricSpaceExtensions::runRangeQuery(dataset, distanceType, dataType);
                    break;

                case 3:
                    cout << "\n=== ִ�о������޵� kNN ��ѯ ===\n";
                    runBoundedKnnQuery(dataset, distanceType, dataType);
                    break;

                case 4:
                    cout << "\n=== ִ������ڲ�ѯ ===\n";
                    runNearestNeighborQuery(dataset, distanceType, dataType);
                    break;

                case 0:
                    cout << "�˳�����...\n";
                    break;

                default:
                    cout << "��Чѡ�������ѡ��\n";
                }
                // ���в�ѯ
                runNearestNeighbor(dataset, distanceType, dataType);
            }
            else {
                // ���е����뺯����ѯʵ��
                runExperiments(dataset, data_num, distanceType, dataType);
            }
        }

        cout << "����0���������̣�����1�����¿�ʼ\n";
        cin >> systemBegin;
        cout << "------------------------------------------------\n";
    }
}




