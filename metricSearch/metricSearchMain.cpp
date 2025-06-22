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
            //ѡ��ϵͳģʽ
            int systemSet = 0;
            cout << "��ѡ��ϵͳģʽ��\n1.��ѯģʽ��չʾ��֧ͬ�ŵ㡢��ͬ���뺯���Ĳ�ѯ���ܣ�\n2.��ѯģʽ�����û�ѡ��֧�ŵ㡢��ѯ�㡢���뺯��" << endl;
            cin >> systemSet;

            //�������ݼ�
            int data_num = 0;  //���ݼ�����
            int data_var = 0;  //���ݼ�����
            cout << "������Ҫ��������ݼ����ͣ�1��������2.�ַ�����3.�����ʣ�" << endl;
            cin >> data_var;
            cout << "������Ҫ��ȡ�����ݸ���������С��3����" << endl;
            cin >> data_num;
            if (data_num < 3) {
                cout << "���ݼ���Ҫ����3���������ܽ���ʵ��" << endl;
                return 1;
            }

            // �������ݼ�
            auto dataset = loadUMADData(data_var, data_num);

            //ѡ����뺯��
            int distanceType = 0;
            if (data_var == 1) {
                cout << "��ѡ��ʹ�õľ��뺯��" << endl;
                cout << "1.EuclideanDistance\n2.ChebyshevDistance\n3.LonePointDistance\n4.ManhattanDistance\n��ע������0����ȫ���뺯����ѯ��";// 0=���У�1/2/3=ָ������
            }
            else if (data_var == 2) {
                cout << "��ѡ��ʹ�õľ��뺯��" << endl;
                cout << "1.EditDistance\n2.HammingDistance\n";
            }
            else if (data_var == 3) {
                cout << "��ѡ��ʹ�õľ��뺯��" << endl;
                cout << "1.WeightedEditDistance\n";
            }
            cin >> distanceType;

            //�����0������ȫ���뺯����ѯ�����򣬽��в�ѯģʽ�򵥾��뺯����ѯ
            if (distanceType == 0) {

            }
            else {
                //��ȡ���뺯��
                auto func = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);
                //Ԥ���������뺯��
                MetricSpaceSearch::precomputeDistances(dataset, func);
                if (systemSet == 2) {
                    // ���в�ѯ
                    runSearch(dataset, distanceType, data_var);
                }
                else {
                    // ���е����뺯����ѯʵ��
                    runExperiments(dataset, data_num, distanceType, data_var);
                }
            }
            
            cout << "����0���������̣�����1�����¿�ʼ\n";
            cin >> systemBegin;
            cout << "------------------------------------------------\n";
        }
        
    }
    catch (const exception& e) {
        cout << "����: " << e.what() << endl;
        return 1;
    }

    return 0;
}