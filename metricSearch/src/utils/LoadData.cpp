#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/LoadData.h"
#include "../../include/utils/MetricSpaceSearch.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <stdexcept>

using namespace std;

// 数据集加载函数
DataList loadUMADData(int data_var, int data_num) {
    const std::string vectorFilename = "clusteredvector-2d-100k-100c.txt";      // 向量数据文件
    const std::string stringFilename = "English.dic";       // 字符串数据文件
    const std::string proteinFilename = "yeast.aa";         // 蛋白质数据文件

    DataList result;

    switch (data_var) {
    case 1: { // 向量数据
        auto vecData = loadVectorData(vectorFilename, data_num); // 假设已有该函数
        result.assign(vecData.begin(), vecData.end());
        std::cout << "[加载完成] 已加载向量数据集，共 " << vecData.size() << " 条。\n";
        break;
    }
    case 2: { // 字符串数据
        auto strData = loadStringData(stringFilename, data_num); // 假设已有该函数
        result.assign(strData.begin(), strData.end());
        std::cout << "[加载完成] 已加载字符串数据集，共 " << strData.size() << " 条。\n";
        break;
    }
    case 3: { // 蛋白质序列数据
        auto proteinData = loadProteinData(proteinFilename, data_num);
        result.assign(proteinData.begin(), proteinData.end());
        std::cout << "[加载完成] 已加载蛋白质序列数据集，共 " << proteinData.size() << " 条。\n";
        break;
    }
    default:
        std::cerr << "[错误] 不支持的数据类型: " << data_var << "\n";
    }

    return result;
}

//加载向量数据
vector<shared_ptr<VectorData>> loadVectorData(const string& filename, int num_vectors) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[错误] 无法打开文件: " << filename << endl;
        throw runtime_error("无法打开文件: " + filename);
    }

    cout << "[信息] 正在加载向量数据集: " << filename << endl;

    int dimensions, total_vectors;
    file >> dimensions >> total_vectors;

    cout << "[元数据] 维度: " << dimensions << ", 总向量数: " << total_vectors << endl;

    // 检查 num_vectors 是否合法
    if (num_vectors < 0 || num_vectors > total_vectors) {
        num_vectors = total_vectors;
        cout << "[提示] 将加载全部 " << total_vectors << " 个向量。\n";
    }
    else {
        cout << "[提示] 将加载前 " << num_vectors << " 个向量。\n";
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

    cout << "[完成] 成功加载 " << num_vectors << " 个向量。\n";

    return data;
}

//加载字符串数据
vector<shared_ptr<StringData>> loadStringData(const string& filename, int num_strings = -1) {
    vector<shared_ptr<StringData>> dataList;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[错误] 无法打开文件: " << filename << endl;
        return dataList; // 返回空列表
    }

    cout << "[信息] 正在加载字符串数据集: " << filename << endl;

    string line;
    int lineNumber = 0;
    int successCount = 0;

    while (getline(file, line)) {
        lineNumber++;

        // 忽略空行
        if (line.empty()) continue;

        // 如果指定了 num_strings 且已满足，则跳出循环
        if (num_strings >= 0 && successCount >= num_strings) {
            break;
        }

        // 创建 StringData 对象并加入列表
        try {
            auto data = make_shared<StringData>(line, successCount); // 第二个参数可以是ID或索引
            dataList.push_back(data);
            successCount++;
        }
        catch (...) {
            cerr << "[警告] 第 " << lineNumber << " 行处理失败。\n";
        }
    }

    file.close();

    cout << "[完成] 数据集加载结束，成功读取 " << successCount << " 条字符串。\n";

    return dataList;
}

//加载yeast.aa 文件（蛋白质）
vector<shared_ptr<ProteinData>> loadProteinData(const string& filename, int num_proteins = -1) {
    vector<shared_ptr<ProteinData>> dataList;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[错误] 无法打开文件: " << filename << endl;
        return dataList; // 返回空列表
    }

    cout << "[信息] 正在加载蛋白质序列数据集: " << filename << endl;

    string line;
    string currentSeq;
    int lineNumber = 0;
    int successCount = 0;

    while (getline(file, line)) {
        lineNumber++;

        // 去除前后空白
        line.erase(line.begin(), find_if(line.begin(), line.end(), [](unsigned char ch) {
            return !isspace(ch);
            }));
        line.erase(find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !isspace(ch);
            }).base(), line.end());

        // 忽略空行
        if (line.empty()) continue;

        // 如果是以 '>' 开头，说明是一个新序列的开始
        if (line[0] == '>') {
            // 如果之前已经收集了一个完整的序列，就创建一个 ProteinData 对象
            if (!currentSeq.empty()) {
                try {
                    auto data = make_shared<ProteinData>(currentSeq, successCount++);
                    dataList.push_back(data);
                    currentSeq.clear();

                    // 如果已满足用户指定的数量，则跳出循环
                    if (num_proteins >= 0 && successCount >= num_proteins) {
                        break;
                    }
                }
                catch (...) {
                    cerr << "[警告] 第 " << lineNumber << " 行附近序列处理失败。\n";
                }
            }
            continue;
        }

        // 累加序列内容
        currentSeq += line;
    }

    // 处理最后一个序列
    if (!currentSeq.empty()) {
        try {
            if (num_proteins < 0 || successCount < num_proteins) {
                auto data = make_shared<ProteinData>(currentSeq, successCount++);
                dataList.push_back(data);
            }
        }
        catch (...) {
            cerr << "[警告] 文件末尾附近序列处理失败。\n";
        }
    }

    file.close();

    cout << "[完成] 数据集加载结束，成功读取 " << successCount << " 条蛋白质序列。\n";

    return dataList;
}



