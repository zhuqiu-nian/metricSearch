#ifndef CGHTINTERNALNODE_H
#define CGHTINTERNALNODE_H

#include "../../../include/index_structure/CGHTree/CGHTNode.h"
#include "../../../include/interfaces/MetricData.h"
#include "../../../include/interfaces/MetricDistance.h"
#include <memory>
#include <vector>

using DataPtr = std::shared_ptr<MetricData>;

class CGHTInternalNode : public CGHTNode {
public:
    CGHTInternalNode(
        DataPtr pivot1, DataPtr pivot2, DataPtr pivot3,
        std::vector<std::unique_ptr<CGHTNode>> children,
        std::vector<std::pair<long double, long double>> feature1Ranges,
        std::vector<std::pair<long double, long double>> feature2Ranges,
        std::vector<std::pair<long double, long double>> feature3Ranges,
        std::shared_ptr<MetricDistance> dist);

    std::vector<DataPtr> rangeSearch(const MetricData& q, long double r, long long* distanceCount) override;
    std::vector<DataPtr> getAll() const override;

private:
    DataPtr pivot1_, pivot2_, pivot3_;
    std::vector<std::unique_ptr<CGHTNode>> children_;
    std::vector<std::pair<long double, long double>> feature1Ranges_;  // d1+d2-d3·¶Î§
    std::vector<std::pair<long double, long double>> feature2Ranges_;  // d1-d2+d3·¶Î§
    std::vector<std::pair<long double, long double>> feature3Ranges_;  // d1-d2-d3·¶Î§
    std::shared_ptr<MetricDistance> dist_;
};

#endif // CGHTINTERNALNODE_H