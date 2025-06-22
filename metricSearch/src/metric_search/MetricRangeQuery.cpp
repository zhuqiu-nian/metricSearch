#include "../../include/metric_search/MetricRangeQuery.h"
#include <chrono>
#include <cmath>
#include <iostream>

using namespace std;
using namespace std::chrono;

namespace MetricSpaceExtensions {

    RangeResult rangeQuery(
        const vector<shared_ptr<MetricData>>& dataset,
        const shared_ptr<MetricData>& query,
        const shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const shared_ptr<MetricData>& pivot) {

        RangeResult result;
        auto start = high_resolution_clock::now();

        shared_ptr<MetricData> selectedPivot = pivot ? pivot : dataset[0];
        long double d_pq = distanceFunc->distance(*selectedPivot, *query);
        result.calculations++;

        for (const auto& data : dataset) {
            if (data == query) continue;

            //���ڿ��԰�ȫ���� MetricSpaceSearch::pivotDistanceCache
            auto it_pivot_data = MetricSpaceSearch::pivotDistanceCache.find(selectedPivot.get());
            long double d_pd = 0.0L;

            if (it_pivot_data != MetricSpaceSearch::pivotDistanceCache.end()) {
                auto it_data = it_pivot_data->second.find(data.get());
                if (it_data != it_pivot_data->second.end()) {
                    d_pd = it_data->second;
                }
                else {
                    d_pd = distanceFunc->distance(*selectedPivot, *data);
                    result.calculations++;
                }
            }
            else {
                d_pd = distanceFunc->distance(*selectedPivot, *data);
                result.calculations++;
            }

            long double lower_bound = abs(d_pq - d_pd);
            long double upper_bound = d_pq + d_pd;

            if (lower_bound > radius) {
                result.steps += "���� " + data->toString() + "�����ǲ���ʽ�½� > �뾶��\n";
                continue;
            }

            if (upper_bound <= radius) {
                result.results.push_back(data);
                result.steps += "���� " + data->toString() + "�����ǲ���ʽ�Ͻ� �� �뾶��\n";
                continue;
            }

            long double actual_dist = distanceFunc->distance(*query, *data);
            result.calculations++;
            if (actual_dist <= radius) {
                result.results.push_back(data);
                result.steps += "���� " + data->toString() + "����ʵ���� �� �뾶��\n";
            }
            else {
                result.steps += "�ų� " + data->toString() + "����ʵ���� > �뾶��\n";
            }
        }

        auto end = high_resolution_clock::now();
        result.timeMicrosec = duration_cast<microseconds>(end - start).count();

        return result;
    }

} // namespace MetricSpaceExtensions
