#include "../../include/utils/MetricSpaceSearch.h"
#include <map>
#include <string>

//���ڴ洢����Ĺ�ϣ��
unordered_map<MetricData*, unordered_map<MetricData*, long double>> MetricSpaceSearch::pivotDistanceCache;
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

MetricSpaceSearch::SearchResult MetricSpaceSearch::nearestNeighbor(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    const std::shared_ptr<MetricData>& query,
    const std::shared_ptr<MetricDistance>& distanceFunc,
    const std::shared_ptr<MetricData>& pivot)
{
    const long double EPS = 1e-15;

    SearchResult result;
    result.pivot = pivot;
    result.distanceFunc = distanceFunc;
    result.calculations = 0;

    if (dataset.empty()) {
        return result; // ���ݼ�Ϊ�գ�ֱ�ӷ��ؿս��
    }

    // ���δָ��֧�ŵ㣬��Ĭ��ʹ�õ�һ�����ݵ���Ϊ֧�ŵ�
    std::shared_ptr<MetricData> initialPivot = pivot ? pivot : dataset[0];

    // d(p, q)��֧�ŵ㵽��ѯ��ľ��루���ȴӻ����л�ȡ��
    long double d_pq;
    if (pivotDistanceCache.count(initialPivot.get()) &&
        pivotDistanceCache[initialPivot.get()].count(query.get()))
    {
        d_pq = pivotDistanceCache[initialPivot.get()][query.get()];
    }
    else
    {
        cout << "\n----------------[DEBUG]:Ԥ������ȫ----------------\n";
        d_pq = distanceFunc->distance(*initialPivot, *query);
        pivotDistanceCache[initialPivot.get()][query.get()] = d_pq;
    }

    // ��ʼ�������Ϊ�����Ϳ�ָ��
    result.distance = std::numeric_limits<long double>::max();
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
            "����ʼ����Ϊ: " + std::to_string(d_qx));
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
        if (pivotDistanceCache.count(initialPivot.get()) &&
            pivotDistanceCache[initialPivot.get()].count(data.get()))
        {
            d_px = pivotDistanceCache[initialPivot.get()][data.get()];
        }
        else
        {
            d_px = distanceFunc->distance(*initialPivot, *data);
            pivotDistanceCache[initialPivot.get()][data.get()] = d_px;
        }

        // ====== ʹ�����ǲ���ʽ���м�֦ ======
        long double lower_bound = std::abs(d_pq - d_px);   // �½�
        long double upper_bound = d_pq + d_px;             // �Ͻ�

        // Debug �����֦��Ϣ
        result.steps.push_back("[DEBUG] ��ǰ��: " + data->toString() +
            ", d_px=" + std::to_string(d_px) +
            ", lower_bound=" + std::to_string(lower_bound) +
            ", upper_bound=" + std::to_string(upper_bound) +
            ", current_min=" + std::to_string(result.distance));

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
            " ��ʵ�ʾ���: " + std::to_string(d_qx));

        // ���������
        if (d_qx < result.distance) {
            result.distance = d_qx;
            result.nearest = data;
            result.steps.push_back("�����µ������: " + data->toString() +
                " �������Ϊ " + std::to_string(d_qx));
        }
    }

    //����ж�֧�ŵ��ǲ�������ڣ���Ϊ֮ǰ������֧�ŵ�
    if (d_pq < result.distance) {
        result.distance = d_pq;
        result.nearest = initialPivot;
        result.steps.push_back("�����µ������: ��֧�ŵ㣡�������Ϊ " + std::to_string(d_pq));
    }


    return result;
}

//���ܷ���
void MetricSpaceSearch::analyzePerformance(
    const vector<shared_ptr<MetricData>>& dataset,
    const vector<vector<SearchResult>>& allResults,
    const shared_ptr<MetricData>& query) {

    cout << "���ԣ�����������������ݼ���С=" << dataset.size()
        << "���������=" << allResults.size() << endl;

    if (dataset.empty() || allResults.empty()) return;

    cout << "\n===== �ۺ����ܷ��� =====" << endl;
    cout << "��ѯ��: " << query->toString() << endl;

    struct PivotStat {
        shared_ptr<MetricData> pivot;
        map<string, long double> avgFilterRates;
        map<string, long double> avgTimes;
        map<string, int> totalCalcs;
    };
    vector<PivotStat> pivotStats;

    const int maxPossibleCalcs = dataset.size();

    // �ռ�ͳ������
    for (const auto& roundResults : allResults) {
        if (roundResults.empty()) continue;

        auto pivot = roundResults[0].pivot;
        bool found = false;

        PivotStat* stat = nullptr;
        for (auto& ps : pivotStats) {
            if (ps.pivot == pivot) {
                stat = &ps;
                found = true;
                break;
            }
        }

        if (!found) {
            pivotStats.push_back({ pivot });
            stat = &pivotStats.back();
        }

        for (const auto& result : roundResults) {
            string funcName = result.distanceFunc->getName();

            if (stat->avgFilterRates.find(funcName) == stat->avgFilterRates.end()) {
                stat->avgFilterRates[funcName] = 0.0;
                stat->avgTimes[funcName] = 0.0;
                stat->totalCalcs[funcName] = 0;
            }

            stat->avgFilterRates[funcName] += 1.0 - (long double)(result.calculations) / maxPossibleCalcs;
            stat->avgTimes[funcName] += result.timeMicrosec;
            stat->totalCalcs[funcName] += result.calculations;
        }
    }

    // ƽ����
    for (auto& stat : pivotStats) {
        int rounds = allResults.size();

        // �滻�ṹ���󶨣�for (auto& [name, count] : stat.totalCalcs)
        for (const auto& pair : stat.totalCalcs) {
            const string& name = pair.first;
            stat.avgFilterRates[name] /= rounds;
            stat.avgTimes[name] /= rounds;
        }
    }

    // ����ȽϽ��
    cout << "\n1. ֧�ŵ�Ч�ʶԱ�:" << endl;
    for (const auto& stat : pivotStats) {
        cout << "֧�ŵ� " << stat.pivot->toString() << ":" << endl;

        // �滻�ṹ���󶨣�for (const auto& [name, rate] : stat.avgFilterRates)
        for (const auto& pair : stat.avgFilterRates) {
            const string& name = pair.first;
            long double rate = pair.second;

            cout << "  [" << name << "]"
                << " | ƽ������Ч��: " << rate * 100 << "%"
                << " | ƽ����ʱ: " << stat.avgTimes.at(name) << " ��s"
                << " | �ܼ������: " << stat.totalCalcs.at(name) << "/" << maxPossibleCalcs
                << endl;
        }
    }

    // �ҳ�����֧�ŵ�����ž��뺯��
    struct {
        shared_ptr<MetricData> pivot;
        string bestFunc;
        int minTotal = numeric_limits<int>::max();
    } bestOverall;

    for (const auto& stat : pivotStats) {
        // �滻�ṹ���󶨣�for (const auto& [name, total] : stat.totalCalcs)
        for (const auto& pair : stat.totalCalcs) {
            const string& name = pair.first;
            int total = pair.second;

            if (total < bestOverall.minTotal) {
                bestOverall.pivot = stat.pivot;
                bestOverall.bestFunc = name;
                bestOverall.minTotal = total;
            }
        }
    }

    cout << "\n2. ����֧�ŵ�: " << bestOverall.pivot->toString()
        << " ʹ�� \"" << bestOverall.bestFunc << "\""
        << " (���ټ������: " << bestOverall.minTotal << ")" << endl;

    // ���뺯���Ա�
    map<string, int> totalCalculations;
    for (const auto& roundResults : allResults) {
        for (const auto& result : roundResults) {
            string name = result.distanceFunc->getName();
            totalCalculations[name] += result.calculations;
        }
    }

    cout << "\n3. ���뺯���Ա�:" << endl;
    // �滻�ṹ���󶨣�for (const auto& [name, total] : totalCalculations)
    for (const auto& pair : totalCalculations) {
        const string& name = pair.first;
        int total = pair.second;

        cout << "  [" << name << "] �ܼ������: " << total << endl;
    }

    auto bestFunc = min_element(totalCalculations.begin(), totalCalculations.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second < b.second;
        });

    cout << "�Ƽ�ʹ��: " << bestFunc->first << endl;
}




