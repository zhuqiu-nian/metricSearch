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
#include <filesystem>     // C++17 起支持
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

// 数据集加载函数（增强版）
DataList loadUMADData(int data_var, int data_num) {
    DataList result;

    // 根据 data_var 确定目录和数据类型
    std::string baseDir = "data";      // 基础路径
    std::string subDir;
    std::string dataType;

    switch (data_var) {
    case 1:
        subDir = "vector";
        dataType = "向量";
        break;
    case 2:
        subDir = "string";
        dataType = "字符串";
        break;
    case 3:
        subDir = "protein";
        dataType = "蛋白质";
        break;
    default:
        std::cerr << "[错误] 不支持的数据类型: " << data_var << "\n";
        return result;
    }

    std::filesystem::path p = fs::absolute(baseDir + "/" + subDir);
    std::string fullPath = p.string();  // 获取绝对路径

    // 检查路径是否存在
    if (!fs::exists(fullPath) || !fs::is_directory(fullPath)) {
        std::cerr << "[错误] 文件夹不存在或不是目录: " << fullPath << "\n";
        return result;
    }

    // 列出目录下所有文件
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(fullPath)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().filename().string());
        }
    }

    if (files.empty()) {
        std::cerr << "[警告] 在 '" << fullPath << "' 中未找到任何文件。\n";
        return result;
    }

    // 显示选项并让用户选择
    std::cout << "[选择数据集] 在 '" << dataType << "' 类型中找到以下文件：\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "  " << i + 1 << ". " << files[i] << "\n";
    }

    int choice;
    do {
        std::cout << "请输入要加载的文件编号 (1-" << files.size() << "): ";
        std::cin >> choice;
    } while (choice < 1 || choice > static_cast<int>(files.size()));

    std::string selectedFile = fullPath + "/" + files[choice - 1];

    // 加载对应数据集
    switch (data_var) {
    case 1: {
        auto vecData = loadVectorData(selectedFile, data_num);
        result.assign(vecData.begin(), vecData.end());
        std::cout << "[加载完成] 已加载向量数据集 '" << files[choice - 1] << "', 共 " << vecData.size() << " 条。\n";
        break;
    }
    case 2: {
        auto strData = loadStringData(selectedFile, data_num);
        result.assign(strData.begin(), strData.end());
        std::cout << "[加载完成] 已加载字符串数据集 '" << files[choice - 1] << "', 共 " << strData.size() << " 条。\n";
        break;
    }
    case 3: {
        auto proteinData = loadProteinData(selectedFile, data_num);
        result.assign(proteinData.begin(), proteinData.end());
        std::cout << "[加载完成] 已加载蛋白质数据集 '" << files[choice - 1] << "', 共 " << proteinData.size() << " 条。\n";
        break;
    }
    }

    return result;
}

//加载向量数据
vector<shared_ptr<VectorData>> loadVectorData(const string& filename, int num_vectors) {
    ifstream file(filename);

    /* 添加调试信息
    std::cout << ">>> 尝试加载文件: " << filename << std::endl;
    std::cout << ">>> 绝对路径: " << std::filesystem::absolute(filename).string() << std::endl;
    std::cout << ">>> 文件是否存在? " << std::filesystem::exists(filename) << std::endl;
    std::cout << ">>> 是普通文件? " << std::filesystem::is_regular_file(filename) << std::endl;*/

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



