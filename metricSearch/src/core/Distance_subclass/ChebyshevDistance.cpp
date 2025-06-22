#include "../../../include/core/Distance_subclass/ChebyshevDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"

// ChebyshevDistance ʵ��
long double ChebyshevDistance::distance(const MetricData& a, const MetricData& b) const {
    const auto& va = dynamic_cast<const VectorData&>(a).getVector();
    const auto& vb = dynamic_cast<const VectorData&>(b).getVector();

    if (va.size() != vb.size()) {
        throw runtime_error("����ά�Ȳ�ƥ��");
    }

    long double max_diff = 0.0;
    for (size_t i = 0; i < va.size(); ++i) {
        long double diff = abs(va[i] - vb[i]);
        if (diff > max_diff) {
            max_diff = diff;
        }
    }
    return max_diff;
}

string ChebyshevDistance::getName() const {
    return "�б�ѩ�����";
}