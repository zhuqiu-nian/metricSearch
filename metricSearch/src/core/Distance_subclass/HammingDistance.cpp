#include "../../../include/core/Distance_subclass/HammingDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"
#include <stdexcept>

long double HammingDistance::distance(const MetricData& a, const MetricData& b) const {
    const auto& strA = dynamic_cast<const StringData&>(a).getString();
    const auto& strB = dynamic_cast<const StringData&>(b).getString();

    if (strA.size() != strB.size()) {
        throw std::invalid_argument("HammingDistance: 字符串长度必须一致");
    }

    int distance = 0;
    for (size_t i = 0; i < strA.size(); ++i) {
        if (strA[i] != strB[i]) {
            ++distance;
        }
    }

    return static_cast<long double>(distance);
}