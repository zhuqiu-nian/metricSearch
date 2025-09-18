#include "../../include/utils/SystemLaunch.h"
#include "../../include/index_structure/all_indexHead.h"
#include "../src/intrinsic_dimension/IntrinsicDimensionEstimator.h"

using std::cin;
using std::cout;
using std::endl;

//������ϵͳ��Ϊ��main������࣬�Ұ����Ƕ���������
int systemLaunch() {
	int systemBegin = 1;
	while (systemBegin) {

		//ѡ��ϵͳģʽ����������һ������ʱ�ϳ�
		int systemSet = 0;
		cout << "��ѡ��ϵͳģʽ��\n1.��ѯģʽ��չʾ��֧ͬ�ŵ㡢��ͬ���뺯���Ĳ�ѯ���ܣ�\n2.��ѯģʽ�����û�ѡ��֧�ŵ㡢��ѯ�㡢���뺯��" << endl;
		cin >> systemSet;



		//�������ݼ�
		int data_num = 0;  //���ݼ�����
		int dataType = 0;  //���ݼ�����
		cout << "������Ҫ��������ݼ����ͣ�1��������2.�ַ�����3.�����ʣ�" << endl;
		cin >> dataType;
		cout << "������Ҫ��ȡ�����ݸ���������С��3����" << endl;
		cin >> data_num;
		if (data_num < 3) {
			cout << "���ݼ���Ҫ����3���������ܽ���ʵ��" << endl;
			return 1;
		}

		// �������ݼ�
		auto dataset = loadUMADData(dataType, data_num);

		//ѡ����뺯��
		int distanceType = 0;
		if (dataType == 1) {
			cout << "��ѡ��ʹ�õľ��뺯��" << endl;
			cout << "1.EuclideanDistance\n2.ChebyshevDistance\n3.LonePointDistance\n4.ManhattanDistance\n��ע������0����ȫ���뺯����ѯ��";// 0=���У�1/2/3=ָ������
		}
		else if (dataType == 2) {
			cout << "��ѡ��ʹ�õľ��뺯��" << endl;
			cout << "1.EditDistance\n2.HammingDistance\n";
		}
		else if (dataType == 3) {
			cout << "��ѡ��ʹ�õľ��뺯��" << endl;
			cout << "1.WeightedEditDistance\n";
		}
		cin >> distanceType;

		//ѡ�������ṹ
		int index_var = 0;
		cout << "��ѡ�������ṹ��\n1.����ɨ�裻\n2.Pivot Table\n3.General Hyper-plane Tree\n4.Vantage Point Tree\n5.Multiple Vantage Point Tree" << endl;
		cin >> index_var;

		//ѡ��֧�ŵ�������㷽��
		int pivot_var = 0;
		cout << "��ѡ��֧�ŵ�������㷽��\n1.���ھ�ֵ�ͷ���\n2.���ڷ�Χ��ѯ + ���Իع�\n3.���ھ������ PCA ����ֵ\n";
		cin >> pivot_var;

		int pivotNum = IntrinsicDimensionEstimator::selectOptimalK(dataset, pivot_var, data_num, dataType, distanceType);



		//1.����ɨ��
		if (index_var == 1) {
			//ѡ���ѯ�㷨��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
			cout << "��ѡ�������Բ�ѯ��ʽ��1.k-����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ;4.����ڲ�ѯ\n";
			int search_var;
			cin >> search_var;

			//��ȡ���뺯��
			auto func = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);

			//Ԥ���������뺯��
			MetricSpaceSearch::precomputeDistances(dataset, func);
			switch (search_var)
			{
			case 1:
				cout << "\n=== ִ����ͨ kNN ��ѯ ===\n";
				runKnnQuery(dataset, distanceType, dataType);
				break;

			case 2:
				cout << "\n=== ִ�з�Χ��ѯ ===\n";
				MetricSpaceExtensions::runRangeQuery(dataset, distanceType, dataType);
				break;

			case 3:
				cout << "\n=== ִ�о������޵� kNN ��ѯ ===\n";
				runBoundedKnnQuery(dataset, distanceType, dataType);
				break;

			case 4:
				cout << "\n=== ִ������ڲ�ѯ ===\n";
				runNearestNeighborQuery(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "�˳�����...\n";
				break;

			default:
				cout << "��Чѡ�������ѡ��\n";
			}
		}

		else if (index_var == 2) {
			//ѡ���ѯ�㷨��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
			cout << "��ѡ�������Բ�ѯ��ʽ��1.��Χ��ѯ\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== ִ�з�Χ��ѯ ===\n";
				PivotTable::interactiveRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "�˳�����...\n";
				break;

			default:
				cout << "��Чѡ�������ѡ��\n";
			}
		}

		//GHT
		else if (index_var == 3) {
			//ѡ���ѯ�㷨��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
			cout << "��ѡ�������Բ�ѯ��ʽ��1.��Χ��ѯ\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== ִ�з�Χ��ѯ ===\n";
				GHTree::runGHTRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "�˳�����...\n";
				break;

			default:
				cout << "��Чѡ�������ѡ��\n";
			}
		}

		//VPT
		else if (index_var == 4) {
			//ѡ���ѯ�㷨��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
			cout << "��ѡ�������Բ�ѯ��ʽ��1.��Χ��ѯ\n";
			int search_var;
			cin >> search_var;

			switch (search_var)
			{
			case 1:
				cout << "\n=== ִ�з�Χ��ѯ ===\n";
				VPTree::runVPTRangeSearch(dataset, distanceType, dataType);
				break;

			case 0:
				cout << "�˳�����...\n";
				break;

			default:
				cout << "��Чѡ�������ѡ��\n";
			}
		}

		//MVPT
		else if (index_var == 5) {
		//ѡ���ѯ�㷨��1.����ڲ�ѯ��2.��Χ��ѯ��3.�������޵�k-����ڲ�ѯ
		cout << "��ѡ�������Բ�ѯ��ʽ��1.��Χ��ѯ\n";
		int search_var;
		cin >> search_var;

		switch (search_var)
		{
		case 1:
			cout << "\n=== ִ�з�Χ��ѯ ===\n";
			MVPTree::runMVPTRangeSearch(dataset, distanceType, dataType);
			break;

		case 0:
			cout << "�˳�����...\n";
			break;

		default:
			cout << "��Чѡ�������ѡ��\n";
		}
		}

		cout << "����0���������̣�����1�����¿�ʼ\n";
		cin >> systemBegin;
		cout << "------------------------------------------------\n";
	}

	return 0;
}
