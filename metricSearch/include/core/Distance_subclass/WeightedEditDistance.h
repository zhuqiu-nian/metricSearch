#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../../../include/core/Data_subclass/StringData.h"
#include <unordered_map>

class WeightedEditDistance : public MetricDistance {
    std::unordered_map<char, double> charWeights_;
public:
    explicit WeightedEditDistance(const std::unordered_map<char, double>& weights)
        : charWeights_(weights) {}

    double distance(const MetricData& a, const MetricData& b) const override {
        const auto& strA = dynamic_cast<const StringData&>(a).getString();
        const auto& strB = dynamic_cast<const StringData&>(b).getString();

        const size_t m = strA.size(), n = strB.size();
        std::vector<std::vector<double>> dp(m + 1, std::vector<double>(n + 1, 0));

        for (size_t i = 0; i <= m; ++i) dp[i][0] = i * getWeight(strA[i - 1]);
        for (size_t j = 0; j <= n; ++j) dp[0][j] = j * getWeight(strB[j - 1]);

        for (size_t i = 1; i <= m; ++i) {
            for (size_t j = 1; j <= n; ++j) {
                double cost = (strA[i - 1] == strB[j - 1]) ? 0 :
                    getWeight(strA[i - 1]) + getWeight(strB[j - 1]);

                dp[i][j] = std::min({
                    dp[i - 1][j] + getWeight(strA[i - 1]), // É¾³ýA[i]
                    dp[i][j - 1] + getWeight(strB[j - 1]), // ²åÈëB[j]
                    dp[i - 1][j - 1] + cost                // Ìæ»»
                    });
            }
        }
        return dp[m][n];
    }

    std::string getName() const override { return "¼ÓÈ¨±à¼­¾àÀë"; }

private:
    double getWeight(char c) const {
        return charWeights_.count(c) ? charWeights_.at(c) : 1.0;
    }
};
