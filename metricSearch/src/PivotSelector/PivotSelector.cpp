// PivotSelector.cpp
#include "PivotSelector.h"
#include <algorithm>
#include <random>
#include <limits>
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <Eigen/Dense>

// ---------------------------
// 方法1：最大方差选择
// 每次选使距离向量方差最大的点
// ---------------------------
std::vector<int> PivotSelector::selectByMaxVariance(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist)
{
    int n = allData.size();
    std::vector<int> pivots;
    std::vector<bool> used(n, false);
    std::random_device rd;
    std::mt19937 gen(rd());

    if (k <= 0 || n == 0) return pivots;

    // 随机选第一个（避免全零问题）
    std::uniform_int_distribution<> dis(0, n - 1);
    int first = dis(gen);
    pivots.push_back(first);
    used[first] = true;

    while (pivots.size() < k && pivots.size() < n) {
        double max_variance = -1.0;
        int best_idx = -1;

        for (int candidate = 0; candidate < n; ++candidate) {
            if (used[candidate]) continue;

            // 计算该点到所有已选支撑点的距离向量的方差
            std::vector<double> distances;
            for (int pivot_idx : pivots) {
                double d = dist->distance(*allData[candidate], *allData[pivot_idx]);
                distances.push_back(d);
            }

            double mean = 0.0;
            for (double d : distances) mean += d;
            mean /= distances.size();

            double var = 0.0;
            for (double d : distances) {
                var += (d - mean) * (d - mean);
            }
            var /= distances.size();

            if (var > max_variance) {
                max_variance = var;
                best_idx = candidate;
            }
        }

        if (best_idx != -1) {
            pivots.push_back(best_idx);
            used[best_idx] = true;
        }
        else {
            break; // 无候选
        }
    }

    return pivots;
}

// ---------------------------
// 方法2：最大程度分离
// 选与已选支撑点距离之和最大的点
// ---------------------------
std::vector<int> PivotSelector::selectByMaxSeparation(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist)
{
    int n = allData.size();
    std::vector<int> pivots;
    std::vector<bool> used(n, false);
    std::random_device rd;
    std::mt19937 gen(rd());

    if (k <= 0 || n == 0) return pivots;

    // 随机选第一个
    std::uniform_int_distribution<> dis(0, n - 1);
    int first = dis(gen);
    pivots.push_back(first);
    used[first] = true;

    while (pivots.size() < k && pivots.size() < n) {
        double max_sum_dist = -1.0;
        int best_idx = -1;

        for (int candidate = 0; candidate < n; ++candidate) {
            if (used[candidate]) continue;

            double sum_dist = 0.0;
            for (int pivot_idx : pivots) {
                sum_dist += dist->distance(*allData[candidate], *allData[pivot_idx]);
            }

            if (sum_dist > max_sum_dist) {
                max_sum_dist = sum_dist;
                best_idx = candidate;
            }
        }

        if (best_idx != -1) {
            pivots.push_back(best_idx);
            used[best_idx] = true;
        }
        else {
            break;
        }
    }

    return pivots;
}

// ---------------------------
// 方法3：稀疏空间选择 (Brisaboa et al.)
// 要求新点与已选点的最小距离 >= M * alpha
// ---------------------------
std::vector<int> PivotSelector::selectBySparseSpace(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist,
    double alpha)
{
    int n = allData.size();
    std::vector<int> pivots;
    std::vector<bool> used(n, false);

    if (k <= 0 || n == 0) return pivots;

    // 先计算最大可能距离 M
    double M = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = dist->distance(*allData[i], *allData[j]);
            M = std::max(M, d);
        }
    }
    if (M < 1e-10) M = 1.0; // 防止全相同数据

    double threshold = M * alpha;

    // 随机初始化候选集
    std::vector<int> candidates;
    for (int i = 0; i < n; ++i) candidates.push_back(i);
    std::random_device rd;
    std::shuffle(candidates.begin(), candidates.end(), std::mt19937(rd()));

    for (int idx : candidates) {
        if (pivots.size() >= k) break;

        bool valid = true;
        for (int pivot_idx : pivots) {
            double d = dist->distance(*allData[idx], *allData[pivot_idx]);
            if (d < threshold) {
                valid = false;
                break;
            }
        }

        if (valid) {
            pivots.push_back(idx);
            used[idx] = true;
        }
    }

    // 如果不够，补充最大分离法
    while (pivots.size() < k) {
        double max_min_dist = -1.0;
        int best_idx = -1;
        for (int i = 0; i < n; ++i) {
            if (used[i]) continue;
            double min_dist_to_pivots = std::numeric_limits<double>::max();
            for (int p : pivots) {
                double d = dist->distance(*allData[i], *allData[p]);
                min_dist_to_pivots = std::min(min_dist_to_pivots, d);
            }
            if (min_dist_to_pivots > max_min_dist) {
                max_min_dist = min_dist_to_pivots;
                best_idx = i;
            }
        }
        if (best_idx != -1) {
            pivots.push_back(best_idx);
            used[best_idx] = true;
        }
        else {
            break;
        }
    }

    return pivots;
}

// ---------------------------
// 方法4：Farthest-First Traversal (FFT)
// ---------------------------
std::vector<int> PivotSelector::selectByFarthestFirstTraversal(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist)
{
    int n = allData.size();
    if (k <= 0 || k > n) {
        std::cerr << "无效的支撑点数量: " << k << std::endl;
        return {};
    }

    std::vector<int> pivots;
    std::vector<bool> isPivot(n, false);
    std::vector<long double> minDist(n, std::numeric_limits<long double>::max());

    // Step 1: 随机选择第一个支撑点
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, n - 1);
    int first = dis(gen);
    pivots.push_back(first);
    isPivot[first] = true;

    // 初始化到第一个支撑点的距离
    for (int i = 0; i < n; ++i) {
        if (i != first) {
            minDist[i] = dist->distance(*allData[i], *allData[first]);
        }
    }

    // Step 2: 选择剩余 k-1 个支撑点
    for (int iter = 1; iter < k; ++iter) {
        // 找到距离支撑点集合最远的点
        int nextPivot = -1;
        long double maxMinDist = -1;

        for (int i = 0; i < n; ++i) {
            if (!isPivot[i] && minDist[i] > maxMinDist) {
                maxMinDist = minDist[i];
                nextPivot = i;
            }
        }

        if (nextPivot == -1) break;

        pivots.push_back(nextPivot);
        isPivot[nextPivot] = true;

        // 更新所有点到支撑点集合的最短距离
        for (int i = 0; i < n; ++i) {
            if (!isPivot[i]) {
                long double d = dist->distance(*allData[i], *allData[nextPivot]);
                if (d < minDist[i]) {
                    minDist[i] = d;
                }
            }
        }
    }

    return pivots;
}

// ---------------------------
// 方法5：Incremental Sampling (Bustos)
// ---------------------------
std::vector<int> PivotSelector::selectByIncrementalSampling(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist,
    double /* alpha unused */)
{
    int n = allData.size();
    if (k <= 0 || k > n) {
        std::cerr << "无效的支撑点数量: " << k << std::endl;
        return {};
    }

    std::vector<int> pivots;
    std::vector<bool> isPivot(n, false);

    // 参数设置
    int A = std::min(1000, n * (n - 1) / 2);  // 评估点对数量
    int N = std::min(100, n);                  // 候选集大小

    // Step 1: 使用 FFT 生成候选集 setN
    auto candidateIndices = selectByFarthestFirstTraversal(allData, N, dist);
    std::vector<bool> inCandidate(n, false);
    for (int idx : candidateIndices) {
        inCandidate[idx] = true;
    }

    // Step 2: 随机生成评估集 setA（A 个点对）
    std::vector<std::pair<int, int>> setA;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, n - 1);

    while (setA.size() < A) {
        int i = dis(gen);
        int j = dis(gen);
        if (i != j) {
            setA.emplace_back(i, j);
        }
    }

    // Step 3: 选择第一个支撑点：最大方差
    std::vector<long double> variances(n, 0.0);
    for (int t = 0; t < n; ++t) {
        std::vector<long double> distances;
        for (int x = 0; x < n; ++x) {
            if (x != t) {
                distances.push_back(dist->distance(*allData[t], *allData[x]));
            }
        }
        long double mean = std::accumulate(distances.begin(), distances.end(), 0.0L) / distances.size();
        long double var = 0;
        for (long double d : distances) {
            var += (d - mean) * (d - mean);
        }
        var /= distances.size();
        variances[t] = var;
    }

    int p1 = std::distance(variances.begin(), std::max_element(variances.begin(), variances.end()));
    pivots.push_back(p1);
    isPivot[p1] = true;

    // Step 4: 增量选择后续支撑点
    for (int iter = 1; iter < k; ++iter) {
        long double bestScore = -1;
        int nextPivot = -1;

        for (int t = 0; t < n; ++t) {
            if (isPivot[t] || !inCandidate[t]) continue;

            // 计算将 t 加入后，setA 中所有点对的 L∞ 距离之和
            long double totalLInf = 0;
            std::vector<int> tempPivots = pivots;
            tempPivots.push_back(t);

            for (const auto& [i, j] : setA) {
                long double lInf = 0;
                for (int p : tempPivots) {
                    long double di = dist->distance(*allData[i], *allData[p]);
                    long double dj = dist->distance(*allData[j], *allData[p]);
                    lInf = std::max(lInf, std::abs(di - dj));
                }
                totalLInf += lInf;
            }

            if (totalLInf > bestScore) {
                bestScore = totalLInf;
                nextPivot = t;
            }
        }

        if (nextPivot == -1) break;

        pivots.push_back(nextPivot);
        isPivot[nextPivot] = true;
    }

    return pivots;
}

// ---------------------------
// 方法6：基于拐点的PCA支撑点选择
// ---------------------------
std::vector<int> PivotSelector::selectByPCAOnCandidates(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist,
    double alpha)
{
    int n = allData.size();
    if (k <= 0 || k > n) {
        std::cerr << "无效的支撑点数量: " << k << std::endl;
        return {};
    }

    // Step 1: 使用 FFT 选出拐点作为候选集
    int k_candidate = static_cast<int>(std::max(10.0, alpha * n)); // alpha 控制候选集大小
    k_candidate = std::min(k_candidate, n);
    k_candidate = std::min(k_candidate, 100); // 上限100，避免过大

    auto candidateIndices = selectByFarthestFirstTraversal(allData, k_candidate, dist);
    /*
    std::cout << "使用 FFT 选出 " << k_candidate << " 个拐点作为PCA候选集\n";
    */

    // Step 2: 构造支撑点空间：每个点 x 映射为 (d(x,c1), d(x,c2), ..., d(x,ck_candidate))
    int d = k_candidate; // 支撑点空间维度
    Eigen::MatrixXd dataMatrix(n, d);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k_candidate; ++j) {
            int c_idx = candidateIndices[j];
            dataMatrix(i, j) = static_cast<double>(
                dist->distance(*allData[i], *allData[c_idx])
                );
        }
    }

    // Step 3: 执行 PCA（中心化 + SVD）
    Eigen::VectorXd mean = dataMatrix.colwise().mean();
    Eigen::MatrixXd centered = dataMatrix.rowwise() - mean.transpose();

    // 使用 SVD 进行 PCA
    Eigen::BDCSVD<Eigen::MatrixXd> svd(centered, Eigen::ComputeThinV);
    Eigen::MatrixXd V = svd.matrixV(); // 右奇异向量 = 主成分方向

    // Step 4: 对前 k 个主成分，选择投影绝对值最大的点
    std::vector<int> pivots;
    std::vector<bool> selected(n, false);

    for (int comp = 0; comp < k && comp < d; ++comp) {
        const Eigen::VectorXd& pc = V.col(comp); // 第 comp 个主成分方向

        int bestIdx = -1;
        double maxProj = -1;

        for (int i = 0; i < n; ++i) {
            if (selected[i]) continue;

            // 计算点 i 在该主成分上的投影（带符号）
            double proj = std::abs(dataMatrix.row(i) * pc); // 使用原始坐标投影

            if (proj > maxProj) {
                maxProj = proj;
                bestIdx = i;
            }
        }

        if (bestIdx != -1) {
            pivots.push_back(bestIdx);
            selected[bestIdx] = true;
        }
    }

    return pivots;
}

// ---------------------------
// 方法7：随机选点
// ---------------------------
std::vector<int> PivotSelector::selectByRandom(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k)
{
    int n = static_cast<int>(allData.size());
    if (k >= n) {
        // 返回全部索引
        std::vector<int> indices(n);
        std::iota(indices.begin(), indices.end(), 0);
        return indices;
    }

    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    indices.resize(k);
    return indices;
}

// ---------------------------
// 主接口
// ---------------------------
std::vector<int> PivotSelector::selectPivots(
    const std::vector<std::shared_ptr<MetricData>>& allData,
    int k,
    const std::shared_ptr<MetricDistance>& dist,
    SelectionMethod method,
    double alpha)
{
    if (allData.empty() || k <= 0) {
        return {};
    }

    int actualK = std::min(k, static_cast<int>(allData.size()));

    switch (method) {
    case MAX_VARIANCE:
        return selectByMaxVariance(allData, k, dist);
    case MAX_SEPARATION:
        return selectByMaxSeparation(allData, k, dist);
    case SPARSE_SPACE:
        return selectBySparseSpace(allData, k, dist, alpha);
    case FARTHEST_FIRST_TRAVERSAL:
        return selectByFarthestFirstTraversal(allData, k, dist);
    case INCREMENTAL_SAMPLING:
        return selectByIncrementalSampling(allData, k, dist, alpha);
    case PCA_ON_CANDIDATES:
        return selectByPCAOnCandidates(allData, k, dist, alpha);
    case RANDOM: //  新增分支
        return selectByRandom(allData, actualK);
    default:
        std::cerr << "未知方法，默认使用最大方差法。\n";
        return selectByMaxVariance(allData, k, dist); // 默认
    }
}

// ---------------------------
// 枚举辅助
// ---------------------------
PivotSelector::SelectionMethod PivotSelector::selectPivotMethodFromUser()
{
    std::cout << "\n请选择支撑点选择算法:\n";
    std::cout << "1. 最大方差法       -- 基于距离向量方差，适应数据分布\n";
    std::cout << "2. 最大分离度法     -- 与已选点距离之和最大\n";
    std::cout << "3. 稀疏空间法       -- 支撑点间距离 ≥ M·α (推荐 α=0.35)\n";
    std::cout << "4. 最远优先遍历法   -- 均匀覆盖数据空间 (FFT)\n";
    std::cout << "5. 增量采样法       -- 优化支撑点空间分辨能力 (Bustos)\n";
    std::cout << "6. 拐点PCA法        -- 在FFT拐点集上运行PCA，选主方向极值点\n";
    std::cout << "7. 随机选点        \n";
    std::cout << "请选择 (1-7): ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        std::cout << "已选择：最大方差法\n";
        return PivotSelector::MAX_VARIANCE;

    case 2:
        std::cout << "已选择：最大分离度法\n";
        return PivotSelector::MAX_SEPARATION;

    case 3:
        std::cout << "已选择：稀疏空间法\n";
        return PivotSelector::SPARSE_SPACE;

    case 4:
        std::cout << "已选择：最远优先遍历法 (FFT)\n";
        return PivotSelector::FARTHEST_FIRST_TRAVERSAL;

    case 5:
        std::cout << "已选择：增量采样法 (Bustos)\n";
        return PivotSelector::INCREMENTAL_SAMPLING;

    case 6:
        std::cout << "已选择：拐点PCA法\n";
        return PivotSelector::PCA_ON_CANDIDATES;

    case 7:
        std::cout << "已选择：随机选点\n";
        return PivotSelector::RANDOM;

    default:
        std::cerr << "无效选择，默认使用【最大方差法】。\n";
        return PivotSelector::MAX_VARIANCE;
    }
}