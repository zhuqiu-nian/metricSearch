#define EIGEN_USE_THREADS

#include "IntrinsicDimensionEstimator.h"
#include "../../include/utils/MetricSpaceSearch.h"  // createDistanceFunction
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <cassert>
#include <Eigen/Dense>

using DataPtr = std::shared_ptr<class MetricData>;
using DataList = std::vector<DataPtr>;

// ---------------------------
// 公共接口
// ---------------------------

int IntrinsicDimensionEstimator::selectOptimalK(
    const DataList& dataset,
    int method,
    int data_num,
    int dataType,
    int distanceType)
{
    if (dataset.empty()) {
        std::cerr << "[警告] 数据集为空，返回默认 k=1\n";
        return 1;
    }

    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

    switch (method) {
    case 1:
        return estimateK_MeanVar(dataset, dist);
    case 2:
        return estimateK_RangeQuery(dataset, dist);
    case 3:
        return estimateK_PCA_FullPivot_Raw(dataset, dist);
    case 4:
        return estimateK_PCA(dataset, dist);
    default:
        std::cerr << "[错误] 不支持的方法编号: " << method << "，使用方法1\n";
        return estimateK_MeanVar(dataset, dist);
    }
}

// ---------------------------
// 方法1：基于均值和方差
// ρ = μ² / (2σ²), k ≈ ρ
// ---------------------------

int IntrinsicDimensionEstimator::estimateK_MeanVar(
    const DataList& dataset,
    const std::shared_ptr<MetricDistance>& dist)
{
    auto distances = computeAllPairwiseDistances(dataset, *dist);

    if (distances.empty()) return 1;

    long double sum = std::accumulate(distances.begin(), distances.end(), 0.0L);
    long double mean = sum / distances.size();

    long double var_sum = 0.0L;
    for (long double d : distances) {
        long double diff = d - mean;
        var_sum += diff * diff;
    }
    long double variance = var_sum / distances.size();

    if (variance == 0) {
        std::cerr << "[警告] 方差为0，数据可能退化\n";
        return 1;
    }

    long double rho = (mean * mean) / (2.0L * variance);
    int k = static_cast<int>(std::round(rho));

    std::cout << "[方法1] 本征维度估计 ρ = " << rho << " → 推荐 k = " << std::max(1, k) << std::endl;
    return std::max(1, k);
}

// ---------------------------
// 方法2：基于范围查询 + 线性回归
// m = c * r^ρ  => log m = ρ log r + log c
// ---------------------------

int IntrinsicDimensionEstimator::estimateK_RangeQuery(
    const DataList& dataset,
    const std::shared_ptr<MetricDistance>& dist)
{
    // 1. 自动选择半径（基于数据尺度）
    std::vector<long double> radii = selectRadii(dataset, dist, 10);   //确定半径个数？
    std::vector<long double> log_r, log_m;

    int n = dataset.size();

    // 2. 对每个半径 r，统计每个点的邻域大小
    for (long double r : radii) {
        std::vector<int> counts;
        int valid_centers = 0;
        long double total_count = 0.0;

        for (int i = 0; i < n; ++i) {
            int count = 0;
            for (int j = 0; j < n; ++j) {
                long double d = static_cast<long double>(
                    dist->distance(*dataset[i], *dataset[j])
                    );
                if (d <= r) {
                    ++count;
                }
            }
            // 排除孤立点（只包含自己）
            if (count > 1) { 
                total_count += count;
                ++valid_centers;
            }
        }

        if (valid_centers == 0) continue;

        long double avg_count = total_count / valid_centers;

        if (avg_count > 1.0) {
            log_r.push_back(std::log(r));    
            log_m.push_back(std::log(avg_count));
        }
    }

    if (log_r.size() < 2) {
        std::cerr << "[警告] 范围查询数据不足（log_r.size() = "
            << log_r.size() << "），使用方法1替代\n";
        return estimateK_MeanVar(dataset, dist);
    }

    // 3. 线性回归：log m = rho * log r + b
    int n_pts = log_r.size();
    long double sum_x = 0.0L, sum_y = 0.0L, sum_xy = 0.0L, sum_x2 = 0.0L;

    for (int i = 0; i < n_pts; ++i) {
        sum_x += log_r[i];
        sum_y += log_m[i];
        sum_xy += log_r[i] * log_m[i];
        sum_x2 += log_r[i] * log_r[i];
    }

    long double rho = (n_pts * sum_xy - sum_x * sum_y) /
        (n_pts * sum_x2 - sum_x * sum_x);

    // 防止数值不稳定
    if (!std::isfinite(rho) || rho < 0.1) {
        std::cerr << "[警告] 回归斜率异常 (rho = " << rho << ")，使用方法1替代\n";
        return estimateK_MeanVar(dataset, dist);
    }

    int k = static_cast<int>(std::round(rho));
    std::cout << "[方法2]";
    for (size_t i = 0; i < radii.size(); ++i) {
        std::cout << "  r=" << radii[i] << ", log_r=" << log_r[i]
            << ", log_m=" << log_m[i] << "\n";
    }
    std::cout << "  回归估计 ρ = " << rho << " → 推荐 k = " << std::max(1, k) << std::endl;

    return std::max(1, k);
}

// ---------------------------
// 方法3：基于距离矩阵 PCA 特征值（三条件独立 + 取最小）
// 分别求出三个条件下的 i，取最小者 + 1 作为 k
// ---------------------------

int IntrinsicDimensionEstimator::estimateK_PCA_FullPivot_Raw(
    const DataList& dataset,
    const std::shared_ptr<MetricDistance>& dist)
{
    int n = dataset.size();
    if (n < 2) return 1;

    // 1. 构造距离矩阵 X (n x n)
    Eigen::MatrixXd X(n, n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            X(i, j) = static_cast<double>(
                dist->distance(*dataset[i], *dataset[j])
                );
        }
    }

    // 2. 中心化（列均值）
    Eigen::VectorXd col_means = X.colwise().mean();
    Eigen::MatrixXd X_centered = X.rowwise() - col_means.transpose();

    // 3. 协方差矩阵 C = (1/n) X_c^T X_c
    Eigen::MatrixXd C = (1.0 / n) * X_centered.transpose() * X_centered;

    // 4. 特征值分解
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(C);
    if (eigensolver.info() != Eigen::Success) {
        std::cerr << "[方法3] PCA 分解失败，退化到方法1\n";
        return estimateK_MeanVar(dataset, dist);
    }

    // 5. 提取特征值（降序）
    Eigen::VectorXd eigenvalues = eigensolver.eigenvalues();
    std::vector<double> lambdas;
    double total_var = 0.0;

    for (int i = n - 1; i >= 0; --i) {
        if (eigenvalues(i) > 1e-10) {
            lambdas.push_back(eigenvalues(i));
            total_var += eigenvalues(i);
        }
    }

    if (lambdas.size() < 2) {
        std::cout << "[方法3] 有效特征值不足，k = 1\n";
        return 1;
    }

    // 归一化特征值（百分比）
    std::vector<double> lambdas_norm;
    for (double lam : lambdas) {
        lambdas_norm.push_back(lam / total_var);
    }

    // -----------------------------
    // 6. 分别计算三个条件下的 i
    // -----------------------------

    int i_ratio = 0;           // argmax(λ_i / λ_{i+1})
    int i_cumvar = 0;          // 最小 i 使得累计 ≥60%
    int i_nextvar = 0;         // 最小 i 使得 λ_{i+1} ∈ [1.5%, 3.5%]
    bool found_nextvar = false;

    double max_ratio = 0.0;
    double cum_var = 0.0;

    for (size_t i = 0; i < lambdas.size() - 1; ++i) {
        // --- 条件1: 最大比例 ---
        double ratio = lambdas[i] / (lambdas[i + 1] + 1e-15);
        if (ratio > max_ratio) {
            max_ratio = ratio;
            i_ratio = static_cast<int>(i);
        }

        // --- 条件2: 累计方差 ≥60% ---
        cum_var += lambdas_norm[i];
        if (cum_var >= 0.6 && i_cumvar == 0) {
            i_cumvar = static_cast<int>(i);
        }

        // --- 条件3: λ_{i+1} ∈ [1.5%, 3.5%] ---
        double lambda_next = lambdas_norm[i + 1];
        if (!found_nextvar && lambda_next >= 0.015 && lambda_next <= 0.035) {
            i_nextvar = static_cast<int>(i);
            found_nextvar = true;
        }
    }

    // 如果某个条件未满足，设为最大可能值（避免主导最小值）
    if (i_cumvar == 0) i_cumvar = static_cast<int>(lambdas.size() - 2);
    if (!found_nextvar) i_nextvar = static_cast<int>(lambdas.size() - 2);

    // -----------------------------
    // 7. 取最小 i，然后 k = i + 1
    // -----------------------------
    int min_i = std::min({ i_ratio, i_cumvar, i_nextvar });
    int k = min_i + 1;
    k = std::max(1, std::min(k, 20));  // 限制范围

    // 输出调试信息
    std::cout << "[方法3] 三条件独立取最小策略:\n";
    std::cout << "  λ_i 归一化: ";
    for (double v : lambdas_norm) std::cout << v << " ";
    std::cout << "\n";
    std::cout << "  i_ratio = " << i_ratio
        << ", i_cumvar = " << i_cumvar
        << ", i_nextvar = " << i_nextvar << "\n";
    std::cout << "  min_i = " << min_i << ", 推荐 k = " << k << "\n";

    return k;
}

// ---------------------------
// 方法4：基于距离矩阵 PCA 特征值-注，这里用了CMDS方法，我完全没弄明白这是什么东东
// ρ ≈ argmax_i (λ_i / λ_{i+1})
// ---------------------------

int IntrinsicDimensionEstimator::estimateK_PCA(
    const DataList& dataset,
    const std::shared_ptr<MetricDistance>& dist)
{
    int n = dataset.size();
    if (n < 2) return 1;

    // 1. 构建距离矩阵 D (n x n)
    Eigen::MatrixXd D(n, n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            D(i, j) = static_cast<double>(
                dist->distance(*dataset[i], *dataset[j])
                );
        }
    }

    // 2. D²
    Eigen::MatrixXd D_sq = D.array().square();

    // 3. 中心化矩阵 H
    Eigen::MatrixXd H = Eigen::MatrixXd::Identity(n, n);
    H -= Eigen::MatrixXd::Constant(n, n, 1.0 / n);

    // 4. Gram 矩阵 G = -1/2 * H * D² * H
    Eigen::MatrixXd G = -0.5 * H * D_sq * H;

    // 5. 特征值分解
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigensolver(G);
    if (eigensolver.info() != Eigen::Success) {
        std::cerr << "[方法4] 特征值分解失败，使用方法1替代\n";
        return estimateK_MeanVar(dataset, dist);
    }

    // 6. 获取特征值（降序排列）
    Eigen::VectorXd eigenvalues = eigensolver.eigenvalues();
    std::vector<double> lambdas;
    for (int i = n - 1; i >= 0; --i) {
        // Eigen 返回升序，我们要降序
        if (eigenvalues(i) > 1e-10) {
            lambdas.push_back(eigenvalues(i));
        }
    }

    if (lambdas.size() < 2) {
        std::cerr << "[方法4] 可用特征值不足，推荐 k=1\n";
        return 1;
    }

    // 7. 计算 argmax_i (λ_i / λ_{i+1})
    double max_ratio = 0.0;
    int best_i = 0;

    for (size_t i = 0; i < lambdas.size() - 1; ++i) {
        double ratio = lambdas[i] / (lambdas[i + 1] + 1e-15);  // 防除0
        if (ratio > max_ratio) {
            max_ratio = ratio;
            best_i = i + 1;  // i+1 是维度数（第1~i+1个主成分）
        }
    }

    // best_i 就是使比值最大的那个 i+1，即本征维度估计
    int k_elbow = best_i;

    // 可选：结合其他方法（如累计方差90%）
    double total_var = std::accumulate(lambdas.begin(), lambdas.end(), 0.0);
    double cum_var = 0.0;
    int k_cumvar = 0;
    for (size_t i = 0; i < lambdas.size(); ++i) {
        cum_var += lambdas[i];
        if (cum_var / total_var >= 0.9) {
            k_cumvar = i + 1;
            break;
        }
    }

    // 综合决策：取最大值，保证足够支撑点
    int final_k = std::max({ 1, k_elbow, k_cumvar });

    std::cout << "[方法4] PCA估计：\n"
        << "  特征值比值法（肘部点）: k = " << k_elbow
        << " (max ratio = " << max_ratio << " at i=" << k_elbow << ")\n"
        << "  累计方差90%所需维度: k = " << k_cumvar << "\n"
        << "  最终推荐 k = " << final_k << std::endl;

    return final_k;
}

// ---------------------------
// 工具函数：计算所有点对距离
// ---------------------------

std::vector<long double> IntrinsicDimensionEstimator::computeAllPairwiseDistances(
    const DataList& dataset,
    const MetricDistance& dist)
{
    std::vector<long double> distances;
    int n = dataset.size();
    distances.reserve(n * (n - 1) / 2);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            distances.push_back(dist.distance(*dataset[i], *dataset[j]));
        }
    }
    return distances;
}

// ---------------------------
// 近似特征值计算（简化版，实际应使用专业库）
// ---------------------------

std::vector<long double> approximateTopEigenvalues(
    const std::vector<std::vector<long double>>& matrix, int top_k)
{
    // 简化：只返回对角线元素（近似）
    std::vector<long double> diag;
    int n = matrix.size();
    for (int i = 0; i < std::min(top_k, n); ++i) {
        diag.push_back(std::abs(matrix[i][i]));
    }
    std::sort(diag.rbegin(), diag.rend());
    return diag;
}


// ---------------------------
// 方法2辅助函数：根据数据尺度选取半径
// ---------------------------

std::vector<long double> IntrinsicDimensionEstimator::selectRadii(
    const DataList& dataset,
    const std::shared_ptr<MetricDistance>& dist,
    int num_radii
) {
    int n = dataset.size();
    if (n < 2) return { 1.0 };

    // 1. 计算所有点对距离（或采样一部分）
    std::vector<long double> all_dists;
    const int max_samples = 10000;  // 避免 O(n²) 太大
    int total_pairs = n * (n - 1) / 2;
    int sample_size = std::min(max_samples, total_pairs);

    if (total_pairs <= max_samples) {
        // 小数据：计算所有距离
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                all_dists.push_back(
                    static_cast<long double>(dist->distance(*dataset[i], *dataset[j]))
                );
            }
        }
    }
    else {
        // 大数据：随机采样点对
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, n - 1);

        for (int i = 0; i < sample_size; ++i) {
            int a = dis(gen), b = dis(gen);
            if (a == b) continue;
            if (a > b) std::swap(a, b);
            all_dists.push_back(
                static_cast<long double>(dist->distance(*dataset[a], *dataset[b]))
            );
        }
    }

    // 2. 排序并取分位数
    std::sort(all_dists.begin(), all_dists.end());
    std::vector<long double> radii;
    for (int i = 1; i <= num_radii; ++i) {
        double p = static_cast<double>(i) / (num_radii + 1);  // p = 1/(k+1), ..., k/(k+1)
        int idx = static_cast<int>(p * all_dists.size());
        idx = std::max(1, std::min(static_cast<int>(all_dists.size()) - 1, idx));
        long double r = all_dists[idx];
        radii.push_back(r);
    }

    // 去重并排序
    std::sort(radii.begin(), radii.end());
    radii.erase(std::unique(radii.begin(), radii.end()), radii.end());

    return radii;
}