// MVPTInternalNode.cpp
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTInternalNode.h"
#include "../../../include/index_structure/MultipleVantagePointTree/MVPTLeafNode.h"
#include "../../../include/interfaces/MetricDistance.h"
#include "../../../include/utils/MetricSpaceSearch.h"
#include <algorithm>

MVPTInternalNode::MVPTInternalNode(
    std::vector<DataPtr> pivots,
    std::vector<std::unique_ptr<MVPTNode>> children,
    std::vector<std::vector<long double>> lowerBounds,
    std::vector<std::vector<long double>> upperBounds,
    std::shared_ptr<MetricDistance> dist)
    : pivots_(std::move(pivots)),
    children_(std::move(children)),
    lowerBounds_(std::move(lowerBounds)),
    upperBounds_(std::move(upperBounds)),
    dist_(std::move(dist)) {}

std::vector<DataPtr> MVPTInternalNode::rangeSearch(const MetricData& q, long double r, long long* distanceCount) {
    std::vector<DataPtr> result;

    // �������֧�ŵ��Ƿ��ڷ�Χ��
    for (const auto& vp : pivots_) {
        long double d = dist_->distance(q, *vp);
        if (distanceCount) (*distanceCount)++;
        if (d <= r) {
            result.push_back(vp);
        }
    }

    // ����ÿ���������Լ�֦
    for (size_t ci = 0; ci < children_.size(); ++ci) {
        bool done = false;

        // ����Ƿ����κ�һ��֧�ŵ������ȫ����������
        for (size_t pi = 0; pi < pivots_.size(); ++pi) {
            long double d_vp_q = dist_->distance(q, *pivots_[pi]);
            if (distanceCount) (*distanceCount)++;

            if (d_vp_q + upperBounds_[pi][ci] <= r) {
                // ����ȫ��������������������
                auto childResult = children_[ci]->getAll();
                result.insert(result.end(), childResult.begin(), childResult.end());
                done = true;
                break;
            }

            if (d_vp_q + r <= lowerBounds_[pi][ci] ||
                d_vp_q - r >= upperBounds_[pi][ci]) {
                // �����������н⣬����
                done = true;
                break;
            }
        }

        if (!done) {
            // �޷���֦���ݹ�����
            auto childResult = children_[ci]->rangeSearch(q, r, distanceCount);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
    }

    return result;
}

DataList MVPTInternalNode::getAll() const {
    DataList all;
    for (const auto& p : pivots_) {
        all.push_back(p);
    }
    for (const auto& child : children_) {
        auto childAll = child->getAll();
        all.insert(all.end(), childAll.begin(), childAll.end());
    }
    return all;
}