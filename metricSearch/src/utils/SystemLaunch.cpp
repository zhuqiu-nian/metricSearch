#include "../../include/utils/SystemLaunch.h"
#include "../../include/index_structure/all_indexHead.h"
#include "../src/intrinsic_dimension/IntrinsicDimensionEstimator.h"


using std::cin;
using std::cout;
using std::endl;

//启动主系统，为了main函数简洁，我把他们独立出来了
int systemLaunch() {
	int systemBegin = 1;
	while (systemBegin) {

		//选择系统模式————这一部分暂时废除
		int systemSet = 0;
		cout << "请选择系统模式：\n1.轮询模式：展示不同支撑点、不同距离函数的查询性能；\n2.查询模式：由用户选择支撑点、查询点、距离函数" << endl;
		cin >> systemSet;

		//读入数据集
		int data_num = 0;  //数据集数量
		int dataType = 0;  //数据集类型
		cout << "请输入要读入的数据集类型（1：向量；2.字符串；3.蛋白质）" << endl;
		cin >> dataType;
		cout << "请输入要读取的数据个数（不得小于3个）" << endl;
		cin >> data_num;
		if (data_num < 3) {
			cout << "数据集需要至少3个向量才能进行实验" << endl;
			return 1;
		}

		// 加载数据集
		auto dataset = loadUMADData(dataType, data_num);

		//选择距离函数
		int distanceType = 0;
		if (dataType == 1) {
			cout << "请选择使用的距离函数" << endl;
			cout << "1.EuclideanDistance\n2.ChebyshevDistance\n3.LonePointDistance\n4.ManhattanDistance\n（注：输入0进行全距离函数轮询）";// 0=所有；1/2/3=指定单个
		}
		else if (dataType == 2) {
			cout << "请选择使用的距离函数" << endl;
			cout << "1.EditDistance\n2.HammingDistance\n";
		}
		else if (dataType == 3) {
			cout << "请选择使用的距离函数" << endl;
			cout << "1.WeightedEditDistance\n";
		}
		cin >> distanceType;

		//选择索引结构
		int index_var = 0;
		cout << "请选择索引结构：\n1.线性扫描；\n2.Pivot Table\n3.General Hyper-plane Tree\n4.Vantage Point Tree\n5.Multiple Vantage Point Tree\n6.Apollonian Tree" << endl;
		cin >> index_var;

		//选择支撑点个数计算方法
		int pivot_var = 0;
		cout << "请选择本征维度估算方法\n1.基于点对距离均值和方差\n2.基于范围查询结果数目和查询半径 \n3.基于距离矩阵 PCA 特征值\n";
		cin >> pivot_var;

		int pivotNum = IntrinsicDimensionEstimator::selectOptimalK(dataset, pivot_var, data_num, dataType, distanceType);



		//1.线性扫描
		if (index_var == 1) {
			//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
			cout << "请选择相似性查询方式：1.k-最近邻查询；2.范围查询；3.距离受限的k-最近邻查询;4.最近邻查询\n";
			int search_var;
			cin >> search_var;

			//获取距离函数
			auto func = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

			//预处理，单距离函数
			MetricSpaceSearch::precomputeDistances(dataset, func);
			switch (search_var)
			{
			case 1:
				cout << "\n=== 执行普通 kNN 查询 ===\n";
				runKnnQuery(dataset, distanceType, dataType);
				break;

			case 2:
				cout << "\n=== 执行范围查询 ===\n";
				MetricSpaceExtensions::runRangeQuery(dataset, distanceType, dataType);
				break;

			case 3:
				cout << "\n=== 执行距离受限的 kNN 查询 ===\n";
				runBoundedKnnQuery(dataset, distanceType, dataType);
				break;

			case 4:
				cout << "\n=== 执行最近邻查询 ===\n";
				runNearestNeighborQuery(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "退出程序...\n";
				break;

			default:
				cout << "无效选项，请重新选择。\n";
			}
		}

		else if (index_var == 2) {
			//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
			cout << "请选择相似性查询方式：1.范围查询\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== 执行范围查询 ===\n";
				PivotTable::interactiveRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "退出程序...\n";
				break;

			default:
				cout << "无效选项，请重新选择。\n";
			}
		}

		//GHT
		else if (index_var == 3) {
			//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
			cout << "请选择相似性查询方式：1.范围查询\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== 执行范围查询 ===\n";
				GHTree::runGHTRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "退出程序...\n";
				break;

			default:
				cout << "无效选项，请重新选择。\n";
			}
		}

		//VPT
		else if (index_var == 4) {
			//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
			cout << "请选择相似性查询方式：1.范围查询\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== 执行范围查询 ===\n";
				VPTree::runVPTRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "退出程序...\n";
				break;

			default:
				cout << "无效选项，请重新选择。\n";
			}
		}

		//MVPT
		else if (index_var == 5) {
		//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
		cout << "请选择相似性查询方式：1.范围查询\n";
		int search_var;
		cin >> search_var;

		switch (search_var)
		{
		case 1:
			cout << "\n=== 执行范围查询 ===\n";
			MVPTree::runMVPTRangeSearch(dataset, distanceType, dataType);
			break;

		case 0:
			cout << "退出程序...\n";
			break;

		default:
			cout << "无效选项，请重新选择。\n";
		}
		}

		//Apollonian Tree
		else if (index_var == 6) {
		//选择查询算法：1.最近邻查询；2.范围查询；3.距离受限的k-最近邻查询
		cout << "请选择相似性查询方式：1.范围查询\n";
		int search_var;
		cin >> search_var;

		switch (search_var)
		{
		case 1:
			cout << "\n=== 执行范围查询 ===\n";
			ApollonianTree::runApollonianRangeSearch(dataset, distanceType, dataType);
			break;

		case 0:
			cout << "退出程序...\n";
			break;

		default:
			cout << "无效选项，请重新选择。\n";
		}
		}

		cout << "输入0来结束进程，输入1来重新开始\n";
		cin >> systemBegin;
		cout << "------------------------------------------------\n";
	}


}
