#pragma once
#include "../../interfaces/MetricDistance.h"
#include "../../../include/core/Data_subclass/StringData.h"
#include <vector>
#include <algorithm>

class EditDistance : public MetricDistance {
public:
    double distance(const MetricData& a, const MetricData& b) const override {
        const auto& strA = dynamic_cast<const StringData&>(a).getString();
        const auto& strB = dynamic_cast<const StringData&>(b).getString();

        // ∂ØÃ¨πÊªÆº∆À„±‡º≠æ‡¿Î
        const size_t m = strA.size(), n = strB.size();
        std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

        for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
        for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

        for (size_t i = 1; i <= m; ++i) {
            for (size_t j = 1; j <= n; ++j) {
                int cost = (strA[i - 1] == strB[j - 1]) ? 0 : 1;
                dp[i][j] = std::min({
                    dp[i - 1][j] + 1,    // …æ≥˝
                    dp[i][j - 1] + 1,    // ≤Â»Î
                    dp[i - 1][j - 1] + cost // ÃÊªª
                    });
            }
        }
        return dp[m][n];
    }

    std::string getName() const override { return "±‡º≠æ‡¿Î"; }
};
