// testCorrectness.h
#ifndef TEST_CORRECTNESS_H
#define TEST_CORRECTNESS_H

/**
 * @brief 执行多索引结构（GHT / MVPT / CGHT / CLPT / AT）的范围搜索正确性验证。
 *
 * 该函数会：
 *   - 加载指定的小型数据集（默认为 2D 向量）
 *   - 构建 GHT、MVPT、CGHT、CLPT 和 Apollonian Tree (AT) 索引
 *   - 对每个索引执行相同的范围查询（以第一个点为查询对象）
 *   - 输出各算法的距离计算次数和匹配结果数量
 *   - 用户可通过比较“匹配项数量”验证实现是否正确（应与暴力搜索一致）
 *
 * @return int 返回值：
 *         - 0 表示成功完成测试
 *         - -1 表示数据加载失败或其他错误
 */
int run_test_correctness();

#endif // TEST_CORRECTNESS_H