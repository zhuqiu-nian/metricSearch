#pragma once

#include "../../interfaces/MetricData.h"
#include "../../interfaces/MetricDistance.h"

#include <vector>
#include <memory>
#include <string>

class PivotTable {
public:
    // �������캯�������� distanceType �� data_var ������
    PivotTable(const std::vector<std::shared_ptr<MetricData>>& allData,
        const std::vector<int>& pivotIndices,
        int distanceType,
        int data_var);

    PivotTable(const vector<shared_ptr<MetricData>>& allData,
        int k,
        int distanceType,
        int data_var);

    // �������ǲ���ʽ�������ӿ�
    std::vector<std::shared_ptr<MetricData>> search(
        const MetricData& query,
        long double threshold,
        long long* distanceCount = nullptr) const;  // ��ѡ������Ĭ��Ϊ nullptr

    // �������������ѡȡ k ��֧�ŵ�����
    vector<int> selectRandomPivots(int totalSize, int k) const;

    //�ⲿ�ӿڣ�����PivotTable������ز�ѯ
    // ������̬����������ʽ��Χ��ѯ
    static void interactiveRangeSearch(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        int distanceType,
        int dataType);

    // ��ȡ�������ڲ�����������
    static long long getDistanceCalculations() { return distanceCalculations_; }
    static void resetDistanceCalculations() { distanceCalculations_ = 0; }

private:
    std::vector<std::shared_ptr<MetricData>> data_;          // ��֧�ŵ�����
    std::vector<std::shared_ptr<MetricData>> pivots_;         // ֧�ŵ㼯��
    std::vector<std::vector<long double>> distanceTable_;     // �����pivots �� data_
    std::shared_ptr<MetricDistance> distance_;                // ��̬ѡ��ľ��뺯��
    vector<int> selectedPivotIndices_;  // ��¼����ѡ�е�֧�ŵ��������������

    // ���������������������ʱ
    void buildDistanceTable(int distanceType, int data_var);

    static long long distanceCalculations_;  // ���������ھ���������ͳ��

};