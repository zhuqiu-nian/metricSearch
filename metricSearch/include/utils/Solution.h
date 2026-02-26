#pragma once
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"
#include < functional >
#include "MetricSpaceSearch.h"

// 实验运行函数
void runExperiments(
    const vector<shared_ptr<MetricData>>& dataset,
    int data_num,
    int enableAllDistances,  // 0=所有；1/2/3=指定单个
    int data_var);            // 1=向量；2=字符串

void runExperimentsWithMultipleDistances(
    const vector<shared_ptr<MetricData>>& dataset,
    int data_num,
    int enableAllDistances,  // 0=所有；1/2/3=指定单个
    int data_var);            // 1=向量；2=字符串;3=蛋白质

// 实验运行函数（查询模式）
void runNearestNeighbor(
    const vector<shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType);

// 自定义输入查询点（返回 shared_ptr<MetricData>）
shared_ptr<MetricData> inputCustomQuery(int dataType);

//统一输出函数逻辑
void printSearchResult(const MetricSpaceSearch::SearchResult& result);

//启动主系统，为了main函数简洁，我把他们独立出来了
int systemLaunch();


