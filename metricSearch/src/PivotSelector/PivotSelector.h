// PivotSelector.h
#pragma once

#include <vector>
#include <memory>
#include "../../include/interfaces/MetricData.h"
#include "../../include/interfaces/MetricDistance.h"

class PivotSelector {
public:
    enum SelectionMethod {
        MAX_VARIANCE,      // 方法1：最大方差
        MAX_SEPARATION,    // 方法2：最大程度分离
        SPARSE_SPACE,      // 方法3：稀疏空间选择 (Brisaboa et al.)
        FARTHEST_FIRST_TRAVERSAL, // 方法4：最远优先遍历 (FFT)
        INCREMENTAL_SAMPLING,    // 方法5：增量采样法 (Bustos)
        PCA_ON_CANDIDATES,      // 方法6：基于候选拐点的PCA
        RANDOM              //随机
    };

    /**
     * @brief 选择支撑点索引
     * @param allData 所有数据点
     * @param k 期望的支撑点个数
     * @param dist 距离函数
     * @param method 选择方法
     * @param alpha 稀疏空间选择的阈值参数 (默认0.35)
     * @return 支撑点索引向量
     */
    static std::vector<int> selectPivots(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        SelectionMethod method = MAX_VARIANCE,
        double alpha = 0.35
    );

    /**
 * @brief 从用户输入中选择支撑点选择算法
 * @return 返回对应的 SelectionMethod 枚举值
 */
    static PivotSelector::SelectionMethod selectPivotMethodFromUser();

private:
    // 方法1：最大方差
    static std::vector<int> selectByMaxVariance(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // 方法2：最大程度分离
    static std::vector<int> selectByMaxSeparation(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // 方法3：稀疏空间选择
    static std::vector<int> selectBySparseSpace(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha);

    // 方法4：最远优先遍历 (FFT)
    static std::vector<int> selectByFarthestFirstTraversal(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist);

    // 方法5：增量采样法 (Bustos)
    static std::vector<int> selectByIncrementalSampling(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha); // alpha 未使用，保留接口一致性

    // 方法6：基于拐点的PCA
    static std::vector<int> selectByPCAOnCandidates(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k,
        const std::shared_ptr<MetricDistance>& dist,
        double alpha); // alpha 用于控制候选集大小（可选）

    //  新增私有方法，随机
    static std::vector<int> selectByRandom(
        const std::vector<std::shared_ptr<MetricData>>& allData,
        int k);
};