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