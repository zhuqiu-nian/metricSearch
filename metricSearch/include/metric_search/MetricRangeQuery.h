#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "../interfaces/MetricData.h"
#include "../interfaces/MetricDistance.h"
#include "../utils/MetricSpaceSearch.h"

namespace MetricSpaceExtensions {

    // ��Χ��ѯ����ṹ��
    struct RangeResult {
        std::vector<std::shared_ptr<MetricData>> results; // �������������ݵ�
        int calculations = 0;                             // ʵ�ʼ���������
        long timeMicrosec = 0;                            // ��ѯ��ʱ��΢�룩
        std::string steps;                                // ��ѯ������־
    };

    // �������ǲ���ʽ�Ķ����ռ䷶Χ��ѯ
    RangeResult rangeQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricData>& query,
        const std::shared_ptr<MetricDistance>& distanceFunc,
        long double radius,
        const std::shared_ptr<MetricData>& pivot = nullptr);

} // namespace MetricSpaceExtensions
