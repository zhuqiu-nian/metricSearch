// test_MVPTandGHT.h
#ifndef BENCHMARK_FOUR_TREES_H
#define BENCHMARK_FOUR_TREES_H

/**
 * @brief 执行多索引结构（GHT / VPT / MVPT / CGHT / CLPT / AT）的批量范围查询性能对比。
 *
 * 该函数会：
 *   - 遍历多个标准数据集
 *   - 对每个数据集构建六种索引结构
 *   - 对每个点执行范围查询（半径固定）
 *   - 统计平均每查询的距离计算次数和耗时（微秒）
 *   - 以表格形式输出结果，便于横向比较
 *
 * 注意：AT（阿波罗尼斯树）使用自适应划分策略，无需手动设置 c1/c2。
 *
 * @return int 返回值：
 *         - 0 表示成功完成 benchmark
 *         - 非 0 表示发生错误（如数据加载失败）
 */
int run_test_VS();

#endif // BENCHMARK_FOUR_TREES_H