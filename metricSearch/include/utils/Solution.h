#pragma once
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"

// 数据集加载函数
vector<shared_ptr<VectorData>> loadUMADData(const string& filename, int num_vectors);

// 实验运行函数
void runExperiments(const vector<shared_ptr<VectorData>>& dataset, int data_num);

//预计算所有支撑点到数据点的距离
static void precomputePivotDistances(
    const vector<shared_ptr<MetricData>>& dataset,
    const shared_ptr<MetricDistance>& distanceFunc);
