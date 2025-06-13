#include "../../include/core/Data_subclass/all_dataHead.h"
#include "../../include/core/Distance_subclass/all_distanceHead.h"
#include "../../include/utils/Solution.h"
#include "../../include/utils/MetricSpaceSearch.h"

using namespace std;


// 数据集加载函数
vector<shared_ptr<VectorData>> loadUMADData(const string& filename, int num_vectors) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("无法打开文件: " + filename);
    }

    int dimensions, total_vectors;
    file >> dimensions >> total_vectors;

    if (num_vectors < 0 || num_vectors > total_vectors) {
        num_vectors = total_vectors;
    }

    vector<shared_ptr<VectorData>> data;
    for (int i = 0; i < num_vectors; ++i) {
        vector<double> vec(dimensions);
        for (int j = 0; j < dimensions; ++j) {
            file >> vec[j];
        }
        data.push_back(make_shared<VectorData>(vec, i));
    }

    return data;
}

// 实验运行函数（轮询）
void runExperiments(const vector<shared_ptr<VectorData>>& dataset, int data_num) {
    auto euclidean = make_shared<EuclideanDistance>();
    auto lonePoint = make_shared<LonePointDistance>();

    // 将VectorData转换为MetricData
    vector<shared_ptr<MetricData>> metricDataset;
    for (const auto& vec : dataset) {
        metricDataset.push_back(static_pointer_cast<MetricData>(vec));
    }

    // 测试所有个向量作为查询
    for (int i = 0; i < data_num && i < metricDataset.size(); ++i) {
        auto query = metricDataset[i];

        cout << "\n\n===== 实验: 查询对象为 " << query->toString() << " =====" << endl;

        MetricSpaceSearch::SearchResult resultEuclidean, resultLonePoint;
        vector<vector<MetricSpaceSearch::SearchResult>> allRoundResults;

        // 测试不同的支撑点
        for (int j = 0; j < data_num && j < metricDataset.size(); ++j) {
            if (i == j) continue; // 避免查询点和支撑点相同

            auto pivot = metricDataset[j];
            vector<MetricSpaceSearch::SearchResult> roundResults;


            cout << "\n--- 使用支撑点: " << pivot->toString() << " ---" << endl;

            // 使用欧几里得距离
            cout << "\n使用欧几里得距离:" << endl;
            auto start = chrono::high_resolution_clock::now();  //记录时间
            resultEuclidean = MetricSpaceSearch::nearestNeighbor(metricDataset, query, euclidean, pivot);
            auto end = chrono::high_resolution_clock::now();
            resultEuclidean.timeMicrosec = chrono::duration_cast<chrono::microseconds>(end - start).count();

            cout << "最近邻: " << resultEuclidean.nearest->toString() << endl;
            cout << "距离: " << resultEuclidean.distance << endl;
            cout << "距离计算次数: " << resultEuclidean.calculations << endl;
            cout << "耗时: "
                << chrono::duration_cast<chrono::microseconds>(end - start).count()
                << " 微秒" << endl;
            cout << "\n[欧几里得距离搜索过程]" << endl;

            // 输出步骤信息
            for (const auto& step : resultEuclidean.steps) {
                cout << "  " << step << endl;
            }

            // 使用孤点距离
            cout << "\n使用孤点距离:" << endl;
            start = chrono::high_resolution_clock::now();
            resultLonePoint = MetricSpaceSearch::nearestNeighbor(metricDataset, query, lonePoint, pivot);
            end = chrono::high_resolution_clock::now();
            resultLonePoint.timeMicrosec = chrono::duration_cast<chrono::microseconds>(end - start).count();

            cout << "最近邻: " << resultLonePoint.nearest->toString() << endl;
            cout << "距离: " << resultLonePoint.distance << endl;
            cout << "距离计算次数: " << resultLonePoint.calculations << endl;
            cout << "耗时: "
                << chrono::duration_cast<chrono::microseconds>(end - start).count()
                << " 微秒" << endl;

            cout << "\n[孤点距离搜索过程]" << endl;

            // 输出步骤信息
            for (const auto& step : resultLonePoint.steps) {
                cout << "  " << step << endl;
            }

            //保存查询数据用于分析
            roundResults.push_back(resultEuclidean);
            roundResults.push_back(resultLonePoint);
            allRoundResults.push_back(roundResults);
        }
        // 每轮实验结束后进行综合分析
        MetricSpaceSearch::analyzePerformance(metricDataset, allRoundResults, query);
    }
}


