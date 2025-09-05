// PivotSelector.h
#pragma once

#include <vector>
#include <memory>
#include "../../include/interfaces/MetricData.h"
#include "../../include/interfaces/MetricDistance.h"

class PivotSelector {
public:
    enum SelectionMethod {
        MAX_VARIANCE,      // ����1����󷽲�
        MAX_SEPARATION,    // ����2�����̶ȷ���
        SPARSE_SPACE,      // ����3��ϡ��ռ�ѡ�� (Brisaboa et al.)
        FARTHEST_FIRST_TRAVERSAL, // ����4����Զ���ȱ��� (FFT)
        INCREMENTAL_SAMPLING,    // ����5������������ (Bustos)
        PCA_ON_CANDIDATES      // ����6�����ں�ѡ�յ��PCA
    };

    /**
     * @brief ѡ��֧�ŵ�����
     * @param allData �������ݵ�
     * @param k ������֧�ŵ����
     * @param dist ���뺯��
     * @param method ѡ�񷽷�
     * @param alpha ϡ��ռ�ѡ�����ֵ���� (Ĭ��0.35)
     * @return ֧�ŵ���������
     */
    static std::vector<int> selectPivots(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        SelectionMethod method = MAX_VARIANCE,
        double alpha = 0.35
    );

    /**
 * @brief ���û�������ѡ��֧�ŵ�ѡ���㷨
 * @return ���ض�Ӧ�� SelectionMethod ö��ֵ
 */
    static PivotSelector::SelectionMethod selectPivotMethodFromUser();

private:
    // ����1����󷽲�
    static std::vector<int> selectByMaxVariance(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // ����2�����̶ȷ���
    static std::vector<int> selectByMaxSeparation(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // ����3��ϡ��ռ�ѡ��
    static std::vector<int> selectBySparseSpace(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha);

    // ����4����Զ���ȱ��� (FFT)
    static std::vector<int> selectByFarthestFirstTraversal(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // ����5������������ (Bustos)
    static std::vector<int> selectByIncrementalSampling(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha); // alpha δʹ�ã������ӿ�һ����

    // ����6�����ڹյ��PCA
    static std::vector<int> selectByPCAOnCandidates(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha); // alpha ���ڿ��ƺ�ѡ����С����ѡ��
};