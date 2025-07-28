#pragma once
#pragma once

#include <vector>
#include <string>
#include "../../include/interfaces/MetricDistance.h"

using DataList = std::vector<std::shared_ptr<MetricData>>;

class IntrinsicDimensionEstimator {
public:
    /**
     * 估算最优支撑点数量 k
     * @param dataset 数据集
     * @param method 方法编号：1=均值方差法, 2=范围查询回归法, 3=PCA特征值法
     * @param data_num 数据数量（可从 dataset.size() 获取，保留为参数兼容旧代码）
     * @param dataType 数据类型（1=向量, 2=字符串, 3=蛋白质）
     * @param distanceType 距离函数类型
     * @return 推荐的支撑点数量 k (≥1)
     */
    static int selectOptimalK(
        const std::vector<std::shared_ptr<class MetricData>>& dataset,
        int method,
        int data_num,
        int dataType,
        int distanceType
    );

    /**
     * @brief 根据数据距离分布自动选择测试半径
     * @param dataset 数据集
     * @param dist 距离函数
     * @param num_radii 期望的半径个数（默认5~7）
     * @return 排序后的半径向量
     */
    static std::vector<long double> selectRadii(
        const DataList& dataset,
        const std::shared_ptr<MetricDistance>& dist,
        int num_radii = 6
    );

private:
    // 方法1：基于点对距离均值和方差
    static int estimateK_MeanVar(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // 方法2：基于范围查询 + 线性回归
    static int estimateK_RangeQuery(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // 方法3：基于距离矩阵的 PCA 特征值
    static int estimateK_PCA_FullPivot_Raw(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // 方法4：基于距离矩阵的 PCA 特征值-CMDS
    static int estimateK_PCA(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const std::shared_ptr<MetricDistance>& dist
    );

    // 工具函数：计算所有点对距离
    static std::vector<long double> computeAllPairwiseDistances(
        const std::vector<std::shared_ptr<MetricData>>& dataset,
        const MetricDistance& dist
    );
};