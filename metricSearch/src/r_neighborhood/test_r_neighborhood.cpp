#include "RNeighborhood.h"
#include "../include/utils/LoadData.h"
#include "../include/utils/MetricSpaceSearch.h"
#include "../include/utils/CLPPartitioner.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

int run_test_r_nei() {
    std::string filename = "data/vector/Uniform-20-d-vector.txt";
    int N = 5000;
    int distanceType = 1; // Euclidean
    int dataType = 1;     // Vector
    auto method = PivotSelector::RANDOM;
    long double queryRadius = 0.02L;

    std::cout << "Loading data: " << filename << " (N=" << N << ")\n";
    auto vecData = loadVectorData(filename, N);
    DataList dataset(vecData.begin(), vecData.end());

    // 选择 3 个 pivots（用于所有方法）
    auto dist = MetricSpaceSearch::createDistanceFunction(distanceType, dataType);
    std::vector<int> pivotIndices = PivotSelector::selectPivots(dataset, 3, dist, method, 0.35);
    DataPtr p1 = dataset[pivotIndices[0]];
    DataPtr p2 = dataset[pivotIndices[1]];
    DataPtr p3 = dataset[pivotIndices[2]];

    // 构建 pivot space 点集（排除 pivots）
    std::vector<std::array<long double, 3>> pivotSpacePoints;
    for (const auto& item : dataset) {
        if (item == p1 || item == p2 || item == p3) continue;
        long double d1 = dist->distance(*p1, *item);
        long double d2 = dist->distance(*p2, *item);
        long double d3 = dist->distance(*p3, *item);
        pivotSpacePoints.push_back({ d1, d2, d3 });
    }

    std::cout << "Pivot space points: " << pivotSpacePoints.size() << "\n";
    std::cout << "Query radius (for r-neighborhood): " << queryRadius << "\n\n";

    // === MVPT ===
    auto mvptPlanes = extractHyperplanesFromMVPT(dataset, 3, 3, distanceType, dataType, method);
    auto mvptCount = countPointsInRNeighborhood(pivotSpacePoints, mvptPlanes, queryRadius);
    std::cout << "MVPT r-neighborhood count: " << mvptCount << "\n";

    // === CGHT ===
    auto cghtPlanes = extractHyperplanesFromCGHT(dataset, 3, distanceType, dataType, method);
    auto cghtCount = countPointsInRNeighborhood(pivotSpacePoints, cghtPlanes, queryRadius);
    std::cout << "CGHT r-neighborhood count: " << cghtCount << "\n";

    /*/ === CLPT (CGH normals) ===
    auto clptPlanes_CGH = extractHyperplanesFromCLPT(dataset, 3, 3, distanceType, dataType, method, "CGH");
    auto clptCount_CGH = countPointsInRNeighborhood(pivotSpacePoints, clptPlanes_CGH, queryRadius);
    std::cout << "CLPT (CGH) r-neighborhood count: " << clptCount_CGH << "\n";
    */

    // === CLPT (PCA normals) ===
    auto clptPlanes_PCA = extractHyperplanesFromCLPT(dataset, 3, 3, distanceType, dataType, method, "PCA");
    auto clptCount_PCA = countPointsInRNeighborhood(pivotSpacePoints, clptPlanes_PCA, queryRadius);
    std::cout << "CLPT (PCA) r-neighborhood count: " << clptCount_PCA << "\n";

    std::cout << "\nLower count => better pruning ability.\n";
    return 0;
}