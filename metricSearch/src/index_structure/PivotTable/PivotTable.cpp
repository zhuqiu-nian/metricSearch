#include "../../../include/index_structure/PivotTable/PivotTable.h"
#include "../../../include/utils/Solution.h"
#include <stdexcept>
#include <limits>
#include <iostream>
#include <chrono>
#include <numeric>
#include <random>

using namespace std;
using namespace std::chrono;

long long PivotTable::distanceCalculations_ = 0;

// �������캯��������֧�ŵ����У�
PivotTable::PivotTable(const vector<shared_ptr<MetricData>>& allData,
    const vector<int>& pivotIndices,
    int distanceType,
    int data_var)
{
<<<<<<< HEAD
    // ������ݼ�Ϊ��
    if (allData.empty()) {
        if (pivotIndices.empty()) {
            buildEmpty(distanceType, data_var);
            return;
        }
        else {
            throw invalid_argument("���ݼ�Ϊ�յ����󹹽�֧�ŵ�");
        }
=======
    if (allData.empty()) {
        throw invalid_argument("���ݼ�Ϊ��");
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    }

    cout << "[Ԥ����] ��ʼ����֧�ŵ����..." << endl;
    cout << "[Ԥ����] ���ݼ���С: " << allData.size() << endl;

<<<<<<< HEAD
    auto start = high_resolution_clock::now();

    // ���˺Ϸ���֧�ŵ�����
    vector<int> validIndices;
    for (int idx : pivotIndices) {
        if (idx >= 0 && idx < static_cast<int>(allData.size())) {
            validIndices.push_back(idx);
        }
        // ��ѡ�����������־
        // else { cout << "[����] ֧�ŵ����� " << idx << " Խ�磬������" << endl; }
    }

    // ���û�кϷ�֧�ŵ㣬�˻�Ϊ�ձ�
    if (validIndices.empty()) {
        buildEmpty(distanceType, data_var);
        return;
    }

    // ��ȡ֧�ŵ�
    for (int idx : validIndices) {
=======
    // ��¼��ʼʱ��
    auto start = high_resolution_clock::now();

    // ��ȡ֧�ŵ�
    for (int idx : pivotIndices) {
        if (idx < 0 || idx >= static_cast<int>(allData.size())) {
            throw out_of_range("pivot index out of range");
        }
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
        pivots_.push_back(allData[idx]);
    }

    // ����֧�ŵ���� data_
    for (size_t i = 0; i < allData.size(); ++i) {
        bool isPivot = false;
<<<<<<< HEAD
        for (int pivotIdx : validIndices) {
=======
        for (int pivotIdx : pivotIndices) {
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
            if (static_cast<size_t>(pivotIdx) == i) {
                isPivot = true;
                break;
            }
        }
        if (!isPivot) {
            data_.push_back(allData[i]);
        }
    }

    // ���������ͬʱ�������뺯����
    buildDistanceTable(distanceType, data_var);

<<<<<<< HEAD
=======
    // ��¼����ʱ��
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "[Ԥ����] ��ɣ�" << endl;
    cout << "[Ԥ����] ������Ŀ��: " << pivots_.size() * data_.size() << endl;
    cout << "[Ԥ����] ��ʱ: " << duration.count() << " ms" << endl;

<<<<<<< HEAD
    selectedPivotIndices_ = validIndices;
=======
    selectedPivotIndices_ = pivotIndices;  // ��¼ѡ�е�����
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
}

// ���������֧�ŵ㵽ÿ����֧�ŵ�ľ���
void PivotTable::buildDistanceTable(int distanceType, int data_var) {

    distance_ = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);  // �������뺯��

<<<<<<< HEAD
    if (pivots_.empty() || data_.empty()) {
        distanceTable_.clear();
        return;
    }

=======
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    distanceTable_.resize(pivots_.size());
    for (size_t p = 0; p < pivots_.size(); ++p) {
        distanceTable_[p].resize(data_.size());
        for (size_t d = 0; d < data_.size(); ++d) {
            distanceTable_[p][d] = distance_->distance(*pivots_[p], *data_[d]);
        }
    }
}

// �������캯��������֧�ŵ������
PivotTable::PivotTable(const vector<shared_ptr<MetricData>>& allData,
<<<<<<< HEAD
    int k,
    int distanceType,
    int data_var)
{
    // ���������
    if (allData.empty()) {
        if (k == 0) {
            buildEmpty(distanceType, data_var);
            return;
        }
        if (k > 0) {
            std::cerr << "[PivotTable] ����: ���ݼ�Ϊ�յ����� k=" << k << "������Ϊ�ձ�" << std::endl;
        }
        buildEmpty(distanceType, data_var);
        return;
    }

    // �Զ����� k
    int actual_k = std::min(k, static_cast<int>(allData.size()));
    if (actual_k <= 0) {
        buildEmpty(distanceType, data_var);
        return;
=======
                       int k,
                       int distanceType,
                       int data_var)
{
    if (allData.size() < static_cast<size_t>(k)) {
        throw invalid_argument("֧�ŵ�������ܴ������ݼ���С");
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    }

    cout << "[Ԥ����] ��ʼ����֧�ŵ����..." << endl;
    cout << "[Ԥ����] ���ݼ���С: " << allData.size() << endl;

    auto start = high_resolution_clock::now();

<<<<<<< HEAD
    // ���ѡȡ֧�ŵ�����
    vector<int> pivotIndices = selectRandomPivots(allData.size(), actual_k);
    selectedPivotIndices_ = pivotIndices;

    // ��ȡ֧�ŵ�
=======
    // �Զ�ѡȡ k ��֧�ŵ㣨�����
    vector<int> pivotIndices = selectRandomPivots(allData.size(), k);
    selectedPivotIndices_ = pivotIndices;

    // ��ȡ֧�ŵ�ͷ�֧�ŵ�
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    for (int idx : pivotIndices) {
        pivots_.push_back(allData[idx]);
    }

<<<<<<< HEAD
    // ��ȡ��֧�ŵ�
=======
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    for (size_t i = 0; i < allData.size(); ++i) {
        bool isPivot = false;
        for (int pivotIdx : pivotIndices) {
            if (static_cast<size_t>(pivotIdx) == i) {
                isPivot = true;
                break;
            }
        }
        if (!isPivot) {
            data_.push_back(allData[i]);
        }
    }

    // ���������
    buildDistanceTable(distanceType, data_var);

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "[Ԥ����] ��ɣ�" << endl;
    cout << "[Ԥ����] ������Ŀ��: " << pivots_.size() * data_.size() << endl;
    cout << "[Ԥ����] ��ʱ: " << duration.count() << " ms" << endl;
}


// �������ǲ���ʽ������
vector<shared_ptr<MetricData>> PivotTable::search(
    const MetricData& query,
    long double threshold,
    long long* distanceCount) const
{
    vector<shared_ptr<MetricData>> result;

<<<<<<< HEAD
    //��������������ݣ�ֱ�ӷ���
    if (data_.empty() && pivots_.empty()) {
        return result;
    }

=======
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
    // ��һ������������֧�ŵ㵽��ѯ����ľ���
    vector<long double> qDists(pivots_.size());
    for (size_t p = 0; p < pivots_.size(); ++p) {
        qDists[p] = distance_->distance(*pivots_[p], query);
        if (distanceCount) (*distanceCount)++;
        else PivotTable::distanceCalculations_++;
    }

    // �ڶ����������������� pivot ֱ�Ӽ�������
    for (size_t p = 0; p < pivots_.size(); ++p) {
        if (qDists[p] <= threshold) {
            result.push_back(pivots_[p]);
        }
    }

    // ������������ÿ����֧�ŵ�����
    for (size_t d = 0; d < data_.size(); ++d) {
        bool done = false;

        for (size_t p = 0; p < pivots_.size(); ++p) {
            long double pivotToQuery = qDists[p];
            long double pivotToData = distanceTable_[p][d];

            // ��������pivotToQuery + pivotToData <= threshold
            if (pivotToQuery + pivotToData <= threshold) {
                result.push_back(data_[d]);
                done = true;
                break;
            }

            // �ų�����abs(pivotToQuery - pivotToData) > threshold
            if (abs(pivotToQuery - pivotToData) > threshold) {
                done = true;
                break;
            }
        }

        // ���޷��жϣ������ʵ�ʾ���
        if (!done) {
            long double actualDist = distance_->distance(*data_[d], query);
            if (distanceCount) (*distanceCount)++;
            else PivotTable::distanceCalculations_++;
            if (actualDist <= threshold) {
                result.push_back(data_[d]);
            }
        }
    }

    return result;
}

vector<int> PivotTable::selectRandomPivots(int totalSize, int k) const {
    vector<int> indices(totalSize);
    iota(indices.begin(), indices.end(), 0);  // 0 ~ totalSize - 1

    // ʹ�����ϴ���㷨��Fisher-Yates Shuffle��
    random_device rd;
    mt19937 g(rd());

    shuffle(indices.begin(), indices.end(), g);

    // ȡǰ k ��
    return vector<int>(indices.begin(), indices.begin() + k);
}

//����Pivot Table

void PivotTable::interactiveRangeSearch(
    const std::vector<std::shared_ptr<MetricData>>& dataset,
    int distanceType,
    int dataType)
{
    if (dataset.empty()) {
        std::cerr << "���ݼ�Ϊ�գ��޷�ִ�в�ѯ��" << std::endl;
        return;
    }

    int pivotCount;
    std::cout << "������֧�ŵ����: ";
    std::cin >> pivotCount;

    if (pivotCount <= 0 || pivotCount >= static_cast<int>(dataset.size())) {
        std::cerr << "֧�ŵ�����������0��С������������" << std::endl;
        return;
    }

    int queryIndex;
    std::cout << "��ѡ���ѯ�������� (0-" << dataset.size() - 1 << "): ";
    std::cin >> queryIndex;

    if (queryIndex < 0 || queryIndex >= static_cast<int>(dataset.size())) {
        std::cerr << "��Ч�Ĳ�ѯ����������" << std::endl;
        return;
    }

    long double threshold;
    std::cout << "�������ѯ�뾶 r: ";
    std::cin >> threshold;

    if (threshold < 0) {
        std::cerr << "��ѯ�뾶����Ϊ������" << std::endl;
        return;
    }

    // �Զ�����֧�ŵ�����
    std::vector<int> indices(dataset.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    std::vector<int> selectedPivots(indices.begin(), indices.begin() + pivotCount);

    // ���� PivotTable
    PivotTable pt(dataset, selectedPivots, distanceType, dataType);

    // ���ü�����
    PivotTable::resetDistanceCalculations();

    // ��ȡ��ѯ����
    auto query = dataset[queryIndex];

    // ִ�з�Χ��ѯ
    auto results = pt.search(*query, threshold);// �����ⲿ������ʱʹ�����ڱ���

    //���˵���ѯ������
    std::vector<std::shared_ptr<MetricData>> filteredResults;
    for (const auto& item : results) {
        if (item.get() != query.get()) {  // �Ƚ�ָ���ַ���ж��Ƿ���ͬһ������
            filteredResults.push_back(item);
        }
    }

    // �����ǰ��ѯ������Ϣ
    std::cout << "\n- ��ѯ���� #" << queryIndex << ": " << dataset[queryIndex]->toString() << std::endl;

    // ������
    std::cout << "\n�ҵ�ƥ������������������ѯ��������: " << filteredResults.size() << std::endl;

    if (!filteredResults.empty()) {
        std::cout << "������ƥ���" << std::endl;
        for (size_t i = 0; i < filteredResults.size(); ++i) {
            std::cout << "  - ƥ���� #" << i + 1 << ": " << filteredResults[i]->toString() << std::endl;
        }
    }
    else {
        std::cout << "δ�ҵ��κ�ƥ���" << std::endl;
    }

    // �������������
    std::cout << "\n���β�ѯ�����þ��뺯��: " << PivotTable::getDistanceCalculations() << " �Σ����� PivotTable �ڲ�ͳ�ƣ�" << std::endl;
<<<<<<< HEAD
}

void PivotTable::buildEmpty(int distanceType, int data_var) {
    // �����������ݣ������Ա�̣�
    data_.clear();
    pivots_.clear();
    distanceTable_.clear();
    selectedPivotIndices_.clear();

    // �������뺯��
    distance_ = MetricSpaceSearch::createDistanceFunction(distanceType, data_var);

    // distanceCalculations_ �� static�������ڴ˳�ʼ��
=======
>>>>>>> 9b3d32b80eaa277037a4b596a70cf11c348ef11d
}