#include "../../include/utils/MetricSpaceSearch.h"

//���ڴ洢����Ĺ�ϣ��
unordered_map<MetricData*, unordered_map<MetricData*, double>> MetricSpaceSearch::pivotDistanceCache;
bool MetricSpaceSearch::isPrecomputed = false;
size_t MetricSpaceSearch::cachedDatasetSize = 0;

//Ԥ��������֧�ŵ㵽���ݵ�ľ���
//��Ԥ��������д��ģ�庯���ˣ��˺���������ͷ�ļ�������

// ������
void MetricSpaceSearch::clearCache() {
    pivotDistanceCache.clear();
    isPrecomputed = false;
    cachedDatasetSize = 0;
    std::cout << "[ϵͳ] ����վ��뻺��" << std::endl;
}

// //�������ǲ���ʽ�Ķ����ռ�����������㷨
MetricSpaceSearch::SearchResult MetricSpaceSearch::nearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricData>& query,
    const shared_ptr<MetricDistance>& distanceFunc,
    const shared_ptr<MetricData>& pivot) {

    SearchResult result;
    result.pivot = pivot;
    result.calculations = 0;

    if (dataset.empty()) {
        return result;
    }

    // ���û��ָ��֧�ŵ㣬ʹ�õ�һ��Ԫ��
    shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // �ӻ����ж�ȡ d_pq��������������㣩
    double d_pq;
    if (pivotDistanceCache.count(initialPivot.get())) {
        d_pq = pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else {
        d_pq = distanceFunc->distance(*initialPivot, *query);
    }

    // ����֧�ŵ�Ϊ��ʼ�����
    result.nearest = initialPivot;
    result.distance = d_pq;

    // �������ݼ�
    for (const auto& data : dataset) {
        if (data == initialPivot) {
            continue; // �Ѿ������
        }

        // Ӧ�����ǲ���ʽ���й���
        double d_px = pivotDistanceCache[initialPivot.get()][data.get()];

        //ע�⣬p�ǲ�ѯ�㣬q��֧�ŵ㣬x�����ݵ㣬�����߼����£�
        //�ȳ�ʼ����ѯ����֧�ŵ����pq������ÿһ�����ݣ��ȼ����֧�ŵ㵽���ݵ�ľ���px��
        if (d_px <= d_pq + result.distance && d_px >= abs(d_pq - result.distance)) {
            // ���ܸ�������Ҫ����ʵ�ʾ���
            double d_qx = distanceFunc->distance(*query, *data);
            result.calculations++;
            result.steps.push_back("�����ѯ�㵽 " +
                data->toString() + " ��ʵ�ʾ���: " + to_string(d_qx));

            if (d_qx < result.distance) {
                result.nearest = data;
                result.distance = d_qx;
                result.steps.push_back("�����µ������: " +
                    data->toString() + " �������Ϊ " +
                    to_string(d_qx));
            }


        }
        else {
            result.steps.push_back("�������ǲ���ʽ�������� " + data->toString() +
                " ����������");
        }
    }

    return result;
}

//���ܷ���
void MetricSpaceSearch::analyzePerformance(
    const vector<shared_ptr<MetricData>>& dataset,
    const vector<vector<SearchResult>>& allResults,
    const shared_ptr<MetricData>& query) {

    //����
    cout << "���ԣ�����������������ݼ���С=" << dataset.size()
        << "���������=" << allResults.size() << endl;



    if (dataset.empty() || allResults.empty()) return;

    cout << "\n===== �ۺ����ܷ��� =====" << endl;
    cout << "��ѯ��: " << query->toString() << endl;

    // ͳ������֧�ŵ�ı���
    struct PivotStat {
        shared_ptr<MetricData> pivot;
        double avgFilterRate;
        double avgTime;
        int totalCalcs;
    };
    vector<PivotStat> pivotStats;

    // �����������������
    const int maxPossibleCalcs = (dataset.size() - 1) * 2 + 1; // (n-1����)*2�־���+�ʼʱ֧�ŵ㵽��ѯ��ľ���

    // �ռ�ͳ������
    // �޸��ռ�ͳ�����ݵĲ���
    for (const auto& roundResults : allResults) {
        if (roundResults.size() != 2) continue;

        const auto& euclideanResult = roundResults[0];
        const auto& lonePointResult = roundResults[1];

        // ֱ�Ӵӽ���л�ȡpivot����Ҫ��nearestNeighbor�����б���pivot��Ϣ��
        auto pivot = euclideanResult.pivot;

        double realFilterRate = 1.0 - (double)(euclideanResult.calculations + lonePointResult.calculations) /
            (maxPossibleCalcs * 2);

        pivotStats.push_back({
            pivot,
            realFilterRate,
            (euclideanResult.timeMicrosec + lonePointResult.timeMicrosec) / 2.0,
            euclideanResult.calculations + lonePointResult.calculations
            });
    }

    // ����ȽϽ��
    cout << "\n1. ֧�ŵ�Ч�ʶԱ�:" << endl;
    for (const auto& stat : pivotStats) {
        cout << "֧�ŵ� " << stat.pivot->toString()
            << " | ƽ������Ч��: " << stat.avgFilterRate * 100 << "%"
            << " | ƽ����ʱ: " << stat.avgTime << " ��s"
            << " | �ܼ������: " << stat.totalCalcs << "/" << maxPossibleCalcs * 2
            << endl;
    }

    // �ҳ�����֧�ŵ�
    auto bestPivot = min_element(pivotStats.begin(), pivotStats.end(),
        [](const PivotStat& a, const PivotStat& b) {
            return a.totalCalcs < b.totalCalcs;
        });

    cout << "\n2. ����֧�ŵ�: " << bestPivot->pivot->toString()
        << " (���ټ������: " << bestPivot->totalCalcs << ")" << endl;

    // ���뺯���Ƚ�
    int euclideanTotal = 0, lonePointTotal = 0;
    for (const auto& roundResults : allResults) {
        euclideanTotal += roundResults[0].calculations;
        lonePointTotal += roundResults[1].calculations;
    }
    cout << "\n3. ���뺯���Ա�:" << endl;
    cout << "ŷ����þ����ܼ������: " << euclideanTotal << endl;
    cout << "�µ�����ܼ������: " << lonePointTotal << endl;
    cout << "�Ƽ�ʹ��: "
        << (euclideanTotal < lonePointTotal ? "ŷ����þ���" : "�µ����")
        << endl;
}

