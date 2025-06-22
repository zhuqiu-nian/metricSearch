#pragma once
#include "../core/Data_subclass/all_dataHead.h"
#include "../core/Distance_subclass/all_distanceHead.h"

using DataList = std::vector<std::shared_ptr<MetricData>>;

DataList loadUMADData(int data_var, int data_num);

//������������
vector<shared_ptr<VectorData>> loadVectorData(const string& filename, int num_vectors);

//�����ַ�������
vector<shared_ptr<StringData>> loadStringData(const string& filename, int num_strings);

//����yeast.aa �ļ��������ʣ�
vector<shared_ptr<ProteinData>> loadProteinData(const string& filename, int num_proteins);
