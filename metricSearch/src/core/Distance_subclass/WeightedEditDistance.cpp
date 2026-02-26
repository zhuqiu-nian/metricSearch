#include "../../../include/core/Distance_subclass/WeightedEditDistance.h"
#include "../../../include/core/Data_subclass/all_dataHead.h"
#include <unordered_map>
#include <cctype> // for std::toupper

/*弄错了
const long double substitutionMatrix[15][15] = {
    {0, 1, 1, 1, 0.5, 1, 0.5, 1, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5}, // A
    {1, 0, 1, 1, 1, 0.5, 0.5, 1, 1, 0.5, 0.5, 1, 0.5, 0.5, 0.5}, // C
    {1, 1, 0, 1, 0.5, 1, 1, 0.5, 1, 0.5, 0.5, 0.5, 1, 0.5, 0.5}, // G
    {1, 1, 1, 0, 1, 0.5, 1, 0.5, 0.5, 1, 0.5, 0.5, 0.5, 1, 0.5}, // T
    {0.5, 1, 0.5, 1, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}, // R
    {1, 0.5, 1, 0.5, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}, // Y
    {0.5, 0.5, 1, 1, 0.5, 0.5, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}, // M
    {1, 1, 0.5, 0.5, 0.5, 0.5, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5}, // K
    {0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 1, 0.5, 0.5, 0.5, 0.5, 0.5}, // W
    {1, 0.5, 0.5, 1, 0.5, 0.5, 0.5, 0.5, 1, 0, 0.5, 0.5, 0.5, 0.5, 0.5}, // S
    {1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.5}, // B
    {0.5, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5, 0.5}, // D
    {0.5, 0.5, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0.5, 0.5}, // H
    {0.5, 0.5, 0.5, 1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0, 0.5}, // V
    {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0}  // N
};*/

const long double substitutionMatrix[21][21] = {
{ 0, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 5, 4, 2, 7 },//A
{ 2, 0, 2, 2, 4, 2, 2, 2, 2, 3, 3, 2, 2, 4, 2, 2, 2, 4, 4, 3, 7 },//R
{ 2, 2, 0, 2, 4, 2, 2, 2, 2, 3, 3, 2, 2, 4, 2, 2, 2, 5, 4, 2, 7 },//N
{ 2, 2, 2, 0, 4, 2, 2, 2, 2, 3, 3, 2, 3, 4, 2, 2, 2, 6, 4, 2, 7 },//D
{ 3, 4, 4, 4, 0, 4, 4, 3, 4, 3, 4, 4, 4, 4, 3, 3, 3, 7, 3, 3, 7 },//C
{ 2, 2, 2, 2, 4, 0, 2, 2, 2, 3, 3, 2, 2, 4, 2, 2, 2, 5, 4, 3, 7 },//Q
{ 2, 2, 2, 2, 4, 2, 0, 2, 2, 3, 3, 2, 3, 4, 2, 2, 2, 6, 4, 2, 7 },//E
{ 2, 2, 2, 2, 3, 2, 2, 0, 2, 2, 3, 2, 2, 4, 2, 2, 2, 6, 4, 2, 7 },//G
{ 2, 2, 2, 2, 4, 2, 2, 2, 0, 3, 3, 2, 3, 3, 2, 2, 2, 5, 3, 3, 7 },//H
{ 2, 3, 3, 3, 3, 3, 3, 2, 3, 0, 1, 3, 2, 2, 2, 2, 2, 5, 3, 2, 7 },//I
{ 2, 3, 3, 3, 4, 3, 3, 3, 3, 1, 0, 3, 1, 2, 3, 3, 2, 4, 2, 1, 7 },//L
{ 2, 2, 2, 2, 4, 2, 2, 2, 2, 3, 3, 0, 2, 4, 2, 2, 2, 4, 4, 3, 7 },//K
{ 2, 2, 2, 3, 4, 2, 3, 2, 3, 2, 1, 2, 0, 2, 2, 2, 2, 4, 3, 2, 7 },//M
{ 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 2, 4, 2, 0, 4, 3, 3, 3, 1, 2, 7 },//F
{ 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3, 2, 2, 4, 0, 2, 2, 5, 4, 2, 7 },//P
{ 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3, 2, 2, 3, 2, 0, 2, 5, 4, 2, 7 },//S
{ 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 0, 5, 3, 2, 7 },//T
{ 5, 4, 5, 6, 7, 5, 6, 6, 5, 5, 4, 4, 4, 3, 5, 5, 5, 0, 4, 5, 7 },//W
{ 4, 4, 4, 4, 3, 4, 4, 4, 3, 3, 2, 4, 3, 1, 4, 4, 3, 4, 0, 3, 7 },//Y
{ 2, 3, 2, 2, 3, 3, 2, 2, 3, 2, 1, 3, 2, 2, 2, 2, 2, 5, 3, 0, 7 },//V
{ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0 } //OTHER
};

// 字符到索引映射
const std::unordered_map<char, int> charToIndex = {
    {'A', 0},  // Alanine
    {'R', 1},  // Arginine
    {'N', 2},  // Asparagine
    {'D', 3},  // Aspartic acid
    {'C', 4},  // Cysteine
    {'Q', 5},  // Glutamine
    {'E', 6},  // Glutamic acid
    {'G', 7},  // Glycine
    {'H', 8},  // Histidine
    {'I', 9},  // Isoleucine
    {'L', 10}, // Leucine
    {'K', 11}, // Lysine
    {'M', 12}, // Methionine
    {'F', 13}, // Phenylalanine
    {'P', 14}, // Proline
    {'S', 15}, // Serine
    {'T', 16}, // Threonine
    {'W', 17}, // Tryptophan
    {'Y', 18}, // Tyrosine
    {'V', 19}, // Valine
    {'X', 20}, // Unknown / Other
};


long double getCost(char a, char b) {
    // 统一转为大写，忽略大小写差异
    a = std::toupper(a);
    b = std::toupper(b);

    // 查找映射表
    auto it_a = charToIndex.find(a);
    auto it_b = charToIndex.find(b);

    // 如果找不到，映射到 'X'（表示未知氨基酸）
    if (it_a == charToIndex.end()) a = 'X';
    if (it_b == charToIndex.end()) b = 'X';

    int i = charToIndex.at(a); // 使用 at() 可以更安全地访问 map
    int j = charToIndex.at(b);

    return substitutionMatrix[i][j];
}

long double WeightedEditDistance::distance(const MetricData& a, const MetricData& b) const {
    const ProteinData* pa = dynamic_cast<const ProteinData*>(&a);
    const ProteinData* pb = dynamic_cast<const ProteinData*>(&b);

    if (!pa || !pb) {
        throw std::invalid_argument("Invalid data type for distance computation");
    }

    const std::string& s = pa->getSequence();
    const std::string& t = pb->getSequence();

    int m = s.size();
    int n = t.size();

    std::vector<std::vector<long double>> dp(m + 1, std::vector<long double>(n + 1));

    dp[0][0] = 0;
    for (int i = 1; i <= m; ++i) dp[i][0] = dp[i-1][0]+7;
    for (int j = 1; j <= n; ++j) dp[0][j] = dp[0][j-1]+7;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            long double cost = getCost(s[i - 1], t[j - 1]);
            dp[i][j] = std::min({
                dp[i - 1][j] + 7,         // 删除
                dp[i][j - 1] + 7,         // 插入
                dp[i - 1][j - 1] + cost   // 替换或匹配
                });
        }
    }

    return dp[m][n];
}