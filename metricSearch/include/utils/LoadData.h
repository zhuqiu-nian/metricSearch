#pragma once
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"

using DataList = std::vector<std::shared_ptr<MetricData>>;

DataList loadUMADData(int data_var, int data_num);

//加载向量数据
vector<shared_ptr<VectorData>> loadVectorData(const string& filename, int num_vectors);

//加载字符串数据
vector<shared_ptr<StringData>> loadStringData(const string& filename, int num_strings);

//加载yeast.aa 文件（蛋白质）
vector<shared_ptr<ProteinData>> loadProteinData(const string& filename, int num_proteins);
