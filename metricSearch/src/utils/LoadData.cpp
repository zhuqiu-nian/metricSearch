#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/LoadData.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <stdexcept>
#include <fstream>
#include <string>
#include <filesystem>     // C++17 ��֧��
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

// ���ݼ����غ�������ǿ�棩
DataList loadUMADData(int data_var, int data_num) {
    DataList result;

    // ���� data_var ȷ��Ŀ¼����������
    std::string baseDir = "data";      // ����·��
    std::string subDir;
    std::string dataType;

    switch (data_var) {
    case 1:
        subDir = "vector";
        dataType = "����";
        break;
    case 2:
        subDir = "string";
        dataType = "�ַ���";
        break;
    case 3:
        subDir = "protein";
        dataType = "������";
        break;
    default:
        std::cerr << "[����] ��֧�ֵ���������: " << data_var << "\n";
        return result;
    }

    std::filesystem::path p = fs::absolute(baseDir + "/" + subDir);
    std::string fullPath = p.string();  // ��ȡ����·��

    // ���·���Ƿ����
    if (!fs::exists(fullPath) || !fs::is_directory(fullPath)) {
        std::cerr << "[����] �ļ��в����ڻ���Ŀ¼: " << fullPath << "\n";
        return result;
    }

    // �г�Ŀ¼�������ļ�
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(fullPath)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().filename().string());
        }
    }

    if (files.empty()) {
        std::cerr << "[����] �� '" << fullPath << "' ��δ�ҵ��κ��ļ���\n";
        return result;
    }

    // ��ʾѡ����û�ѡ��
    std::cout << "[ѡ�����ݼ�] �� '" << dataType << "' �������ҵ������ļ���\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << i + 1 << ". " << files[i] << "\n";
    }

    int choice;
    do {
        std::cout << "������Ҫ���ص��ļ���� (1-" << files.size() << "): ";
        std::cin >> choice;
    } while (choice < 1 || choice > static_cast<int>(files.size()));

    std::string selectedFile = fullPath + "/" + files[choice - 1];

    // ���ض�Ӧ���ݼ�
    switch (data_var) {
    case 1: {
        auto vecData = loadVectorData(selectedFile, data_num);
        result.assign(vecData.begin(), vecData.end());
        std::cout << "[�������] �Ѽ����������ݼ� '" << files[choice - 1] << "', �� " << vecData.size() << " ����\n";
        break;
    }
    case 2: {
        auto strData = loadStringData(selectedFile, data_num);
        result.assign(strData.begin(), strData.end());
        std::cout << "[�������] �Ѽ����ַ������ݼ� '" << files[choice - 1] << "', �� " << strData.size() << " ����\n";
        break;
    }
    case 3: {
        auto proteinData = loadProteinData(selectedFile, data_num);
        result.assign(proteinData.begin(), proteinData.end());
        std::cout << "[�������] �Ѽ��ص��������ݼ� '" << files[choice - 1] << "', �� " << proteinData.size() << " ����\n";
        break;
    }
    }

    return result;
}

//������������
vector<shared_ptr<VectorData>> loadVectorData(const string& filename, int num_vectors) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[����] �޷����ļ�: " << filename << endl;
        throw runtime_error("�޷����ļ�: " + filename);
    }

    cout << "[��Ϣ] ���ڼ����������ݼ�: " << filename << endl;

    int dimensions, total_vectors;
    file >> dimensions >> total_vectors;

    cout << "[Ԫ����] ά��: " << dimensions << ", ��������: " << total_vectors << endl;

    // ��� num_vectors �Ƿ�Ϸ�
    if (num_vectors < 0 || num_vectors > total_vectors) {
        num_vectors = total_vectors;
        cout << "[��ʾ] ������ȫ�� " << total_vectors << " ��������\n";
    }
    else {
        cout << "[��ʾ] ������ǰ " << num_vectors << " ��������\n";
    }

    vector<shared_ptr<VectorData>> data;
    data.reserve(num_vectors);

    for (int i = 0; i < num_vectors; ++i) {
        vector<long double> vec(dimensions);
        for (int j = 0; j < dimensions; ++j) {
            file >> vec[j];
        }

        data.push_back(make_shared<VectorData>(vec, i));
    }

    cout << "[���] �ɹ����� " << num_vectors << " ��������\n";

    return data;
}

//�����ַ�������
vector<shared_ptr<StringData>> loadStringData(const string& filename, int num_strings = -1) {
    vector<shared_ptr<StringData>> dataList;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[����] �޷����ļ�: " << filename << endl;
        return dataList; // ���ؿ��б�
    }

    cout << "[��Ϣ] ���ڼ����ַ������ݼ�: " << filename << endl;

    string line;
    int lineNumber = 0;
    int successCount = 0;

    while (getline(file, line)) {
        lineNumber++;

        // ���Կ���
        if (line.empty()) continue;

        // ���ָ���� num_strings �������㣬������ѭ��
        if (num_strings >= 0 && successCount >= num_strings) {
            break;
        }

        // ���� StringData ���󲢼����б�
        try {
            auto data = make_shared<StringData>(line, successCount); // �ڶ�������������ID������
            dataList.push_back(data);
            successCount++;
        }
        catch (...) {
            cerr << "[����] �� " << lineNumber << " �д���ʧ�ܡ�\n";
        }
    }

    file.close();

    cout << "[���] ���ݼ����ؽ������ɹ���ȡ " << successCount << " ���ַ�����\n";

    return dataList;
}

//����yeast.aa �ļ��������ʣ�
vector<shared_ptr<ProteinData>> loadProteinData(const string& filename, int num_proteins = -1) {
    vector<shared_ptr<ProteinData>> dataList;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[����] �޷����ļ�: " << filename << endl;
        return dataList; // ���ؿ��б�
    }

    cout << "[��Ϣ] ���ڼ��ص������������ݼ�: " << filename << endl;

    string line;
    string currentSeq;
    int lineNumber = 0;
    int successCount = 0;

    while (getline(file, line)) {
        lineNumber++;

        // ȥ��ǰ��հ�
        line.erase(line.begin(), find_if(line.begin(), line.end(), [](unsigned char ch) {
            return !isspace(ch);
            }));
        line.erase(find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !isspace(ch);
            }).base(), line.end());

        // ���Կ���
        if (line.empty()) continue;

        // ������� '>' ��ͷ��˵����һ�������еĿ�ʼ
        if (line[0] == '>') {
            // ���֮ǰ�Ѿ��ռ���һ�����������У��ʹ���һ�� ProteinData ����
            if (!currentSeq.empty()) {
                try {
                    auto data = make_shared<ProteinData>(currentSeq, successCount++);
                    dataList.push_back(data);
                    currentSeq.clear();

                    // ����������û�ָ����������������ѭ��
                    if (num_proteins >= 0 && successCount >= num_proteins) {
                        break;
                    }
                }
                catch (...) {
                    cerr << "[����] �� " << lineNumber << " �и������д���ʧ�ܡ�\n";
                }
            }
            continue;
        }

        // �ۼ���������
        currentSeq += line;
    }

    // �������һ������
    if (!currentSeq.empty()) {
        try {
            if (num_proteins < 0 || successCount < num_proteins) {
                auto data = make_shared<ProteinData>(currentSeq, successCount++);
                dataList.push_back(data);
            }
        }
        catch (...) {
            cerr << "[����] �ļ�ĩβ�������д���ʧ�ܡ�\n";
        }
    }

    file.close();

    cout << "[���] ���ݼ����ؽ������ɹ���ȡ " << successCount << " �����������С�\n";

    return dataList;
}



