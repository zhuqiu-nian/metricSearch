#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/Solution.h"
#include "../../include/utils/MetricSpaceSearch.h"

using namespace std;


// ���ݼ����غ���
vector<shared_ptr<VectorData>> loadUMADData(const string& filename, int num_vectors) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("�޷����ļ�: " + filename);
    }

    int dimensions, total_vectors;
    file >> dimensions >> total_vectors;

    if (num_vectors < 0 || num_vectors > total_vectors) {
        num_vectors = total_vectors;
    }

    vector<shared_ptr<VectorData>> data;
    for (int i = 0; i < num_vectors; ++i) {
        vector<double> vec(dimensions);
        for (int j = 0; j < dimensions; ++j) {
            file >> vec[j];
        }
        data.push_back(make_shared<VectorData>(vec, i));
    }

    return data;
}

// ʵ�����к�������ѯ��
void runExperiments(const vector<shared_ptr<VectorData>>& dataset, int data_num) {
    auto euclidean = make_shared<EuclideanDistance>();
    auto lonePoint = make_shared<LonePointDistance>();

    // ��VectorDataת��ΪMetricData
    vector<shared_ptr<MetricData>> metricDataset;
    for (const auto& vec : dataset) {
        metricDataset.push_back(static_pointer_cast<MetricData>(vec));
    }

    // �������и�������Ϊ��ѯ
    for (int i = 0; i < data_num && i < metricDataset.size(); ++i) {
        auto query = metricDataset[i];

        cout << "\n\n===== ʵ��: ��ѯ����Ϊ " << query->toString() << " =====" << endl;

        MetricSpaceSearch::SearchResult resultEuclidean, resultLonePoint;
        vector<vector<MetricSpaceSearch::SearchResult>> allRoundResults;

        // ���Բ�ͬ��֧�ŵ�
        for (int j = 0; j < data_num && j < metricDataset.size(); ++j) {
            if (i == j) continue; // �����ѯ���֧�ŵ���ͬ

            auto pivot = metricDataset[j];
            vector<MetricSpaceSearch::SearchResult> roundResults;


            cout << "\n--- ʹ��֧�ŵ�: " << pivot->toString() << " ---" << endl;

            // ʹ��ŷ����þ���
            cout << "\nʹ��ŷ����þ���:" << endl;
            auto start = chrono::high_resolution_clock::now();  //��¼ʱ��
            resultEuclidean = MetricSpaceSearch::nearestNeighbor(metricDataset, query, euclidean, pivot);
            auto end = chrono::high_resolution_clock::now();
            resultEuclidean.timeMicrosec = chrono::duration_cast<chrono::microseconds>(end - start).count();

            cout << "�����: " << resultEuclidean.nearest->toString() << endl;
            cout << "����: " << resultEuclidean.distance << endl;
            cout << "����������: " << resultEuclidean.calculations << endl;
            cout << "��ʱ: "
                << chrono::duration_cast<chrono::microseconds>(end - start).count()
                << " ΢��" << endl;
            cout << "\n[ŷ����þ�����������]" << endl;

            // ���������Ϣ
            for (const auto& step : resultEuclidean.steps) {
                cout << "  " << step << endl;
            }

            // ʹ�ùµ����
            cout << "\nʹ�ùµ����:" << endl;
            start = chrono::high_resolution_clock::now();
            resultLonePoint = MetricSpaceSearch::nearestNeighbor(metricDataset, query, lonePoint, pivot);
            end = chrono::high_resolution_clock::now();
            resultLonePoint.timeMicrosec = chrono::duration_cast<chrono::microseconds>(end - start).count();

            cout << "�����: " << resultLonePoint.nearest->toString() << endl;
            cout << "����: " << resultLonePoint.distance << endl;
            cout << "����������: " << resultLonePoint.calculations << endl;
            cout << "��ʱ: "
                << chrono::duration_cast<chrono::microseconds>(end - start).count()
                << " ΢��" << endl;

            cout << "\n[�µ������������]" << endl;

            // ���������Ϣ
            for (const auto& step : resultLonePoint.steps) {
                cout << "  " << step << endl;
            }

            //�����ѯ�������ڷ���
            roundResults.push_back(resultEuclidean);
            roundResults.push_back(resultLonePoint);
            allRoundResults.push_back(roundResults);
        }
        // ÿ��ʵ�����������ۺϷ���
        MetricSpaceSearch::analyzePerformance(metricDataset, allRoundResults, query);
    }
}


