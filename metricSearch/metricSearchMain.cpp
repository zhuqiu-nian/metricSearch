#include "include/core/Data_subclass/all_dataHead.h"
#include "include/core/Distance_subclass/all_distanceHead.h"
#include "include/utils/MetricSpaceSearch.h"
#include "include/utils/Solution.h"
#include <iostream>

using namespace std;

int main() {
    try {
        //1.�������ݼ�
        int data_num = 0;  //���ݼ�����
        cout << "������Ҫ��umad.txt�ж�ȡ�����ݸ���������С��3����" << endl;
        cin >> data_num;
        if (data_num < 3) {
            cout << "���ݼ���Ҫ����3���������ܽ���ʵ��" << endl;
            return 1;
        }
        // �������ݼ�
        auto dataset = loadUMADData("umad.txt", data_num);
        auto d = make_shared<EuclideanDistance>();
        cout << "�����ݼ������� " << dataset.size() << " ������" << endl;
        for (const auto& vec : dataset) {
            cout << vec->toString() << endl;
        }

        //2.Ԥ����
        MetricSpaceSearch::precomputeDistances(dataset, d);

        // ����ʵ��
        runExperiments(dataset, data_num);

    }
    catch (const exception& e) {
        cout << "����: " << e.what() << endl;
        return 1;
    }

    return 0;
}