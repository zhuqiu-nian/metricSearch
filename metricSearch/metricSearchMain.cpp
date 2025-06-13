#include "include/core/Data_subclass/all_dataHead.h"
#include "include/core/Distance_subclass/all_distanceHead.h"
#include "include/utils/MetricSpaceSearch.h"
#include "include/utils/Solution.h"
#include <iostream>

using namespace std;

int main() {
    try {
        //1.读入数据集
        int data_num = 0;  //数据集数量
        cout << "请输入要从umad.txt中读取的数据个数（不得小于3个）" << endl;
        cin >> data_num;
        if (data_num < 3) {
            cout << "数据集需要至少3个向量才能进行实验" << endl;
            return 1;
        }
        // 加载数据集
        auto dataset = loadUMADData("umad.txt", data_num);
        auto d = make_shared<EuclideanDistance>();
        cout << "从数据集加载了 " << dataset.size() << " 个向量" << endl;
        for (const auto& vec : dataset) {
            cout << vec->toString() << endl;
        }

        //2.预处理
        MetricSpaceSearch::precomputeDistances(dataset, d);

        // 运行实验
        runExperiments(dataset, data_num);

    }
    catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }

    return 0;
}