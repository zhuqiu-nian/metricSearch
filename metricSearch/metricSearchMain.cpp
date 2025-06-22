#include "include/core/Data_subclass/all_dataHead.h"
#include "include/core/Distance_subclass/all_distanceHead.h"
#include "include/utils/MetricSpaceSearch.h"
#include "include/utils/Solution.h"
#include "include/utils/LoadData.h"
#include <iostream>

using namespace std;

int main() {
    try {
        int systemBegin = 1;
        while (systemBegin) {
            //选择系统模式
            int systemSet = 0;
            cout << "请选择系统模式：\n1.轮询模式：展示不同支撑点、不同距离函数的查询性能；\n2.查询模式：由用户选择支撑点、查询点、距离函数" << endl;
            cin >> systemSet;

            //读入数据集
            int data_num = 0;  //数据集数量
            int data_var = 0;  //数据集类型
            cout << "请输入要读入的数据集类型（1：向量；2.字符串；3.蛋白质）" << endl;
            cin >> data_var;
            cout << "请输入要读取的数据个数（不得小于3个）" << endl;
            cin >> data_num;
            if (data_num < 3) {
                cout << "数据集需要至少3个向量才能进行实验" << endl;
                return 1;
            }

            // 加载数据集
            auto dataset = loadUMADData(data_var, data_num);

            //选择距离函数
            int distanceType = 0;
            if (data_var == 1) {
                cout << "请选择使用的距离函数" << endl;
                cout << "1.EuclideanDistance\n2.ChebyshevDistance\n3.LonePointDistance\n4.ManhattanDistance\n（注：输入0进行全距离函数轮询）";// 0=所有；1/2/3=指定单个
            }
            else if (data_var == 2) {
                cout << "请选择使用的距离函数" << endl;
                cout << "1.EditDistance\n2.HammingDistance\n";
            }
            else if (data_var == 3) {
                cout << "请选择使用的距离函数" << endl;
                cout << "1.WeightedEditDistance\n";
            }
            cin >> distanceType;

            //如果是0，进行全距离函数轮询，否则，进行查询模式或单距离函数轮询
            if (distanceType == 0) {

            }
            else {
                //获取距离函数
                auto func = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);
                //预处理，单距离函数
                MetricSpaceSearch::precomputeDistances(dataset, func);
                if (systemSet == 2) {
                    // 运行查询
                    runSearch(dataset, distanceType, data_var);
                }
                else {
                    // 运行单距离函数轮询实验
                    runExperiments(dataset, data_num, distanceType, data_var);
                }
            }
            
            cout << "输入0来结束进程，输入1来重新开始\n";
            cin >> systemBegin;
            cout << "------------------------------------------------\n";
        }
        
    }
    catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }

    return 0;
}