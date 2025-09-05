#pragma once
#pragma once

#include <vector>
#include <string>
#include "../../include/interfaces/MetricDistance.h"

using DataList = std::vector<std::shared_ptr<MetricData>>;

class IntrinsicDimensionEstimator {
public:
    /**
     * ��������֧�ŵ����� k
     * @param dataset ���ݼ�
     * @param method ������ţ�1=��ֵ���, 2=��Χ��ѯ�ع鷨, 3=PCA����ֵ��
     * @param data_num �����������ɴ� dataset.size() ��ȡ������Ϊ�������ݾɴ��룩
     * @param dataType �������ͣ�1=����, 2=�ַ���, 3=�����ʣ�
     * @param distanceType ���뺯������
     * @return �Ƽ���֧�ŵ����� k (��1)
     */
    static int selectOptimalK(
        const std::vector<std::shared_ptr<class MetricData>>& dataset,
        int method,
        int data_num,
        int dataType,
        int distanceType
    );

    /**
     * @brief �������ݾ���ֲ��Զ�ѡ����԰뾶
     * @param dataset ���ݼ�
     * @param dist ���뺯��
     * @param num_radii �����İ뾶������Ĭ��5~7��
     * @return �����İ뾶����
     */
    static std::vector<long double> selectRadii(
        const DataList& dataset,
        const std::shared_ptr<MetricDistance>& dist,
        int num_radii = 6
    );

private:
    // ����1�����ڵ�Ծ����ֵ�ͷ���
    static int estimateK_MeanVar(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // ����2�����ڷ�Χ��ѯ + ���Իع�
    static int estimateK_RangeQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // ����3�����ھ������� PCA ����ֵ
    static int estimateK_PCA_FullPivot_Raw(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // ����4�����ھ������� PCA ����ֵ-CMDS
    static int estimateK_PCA(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // ���ߺ������������е�Ծ���
    static std::vector<long double> computeAllPairwiseDistances(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const MetricDistance& dist
    );
};