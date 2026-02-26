// ImageFeatureDistance.cpp
#include "../../../include/core/Distance_subclass/ImageFeatureDistance.h"
#include <stdexcept>
#include <cmath>
#include <cctype>

// ---------- ImageFeatureData 实现 ----------
ImageFeatureData::ImageFeatureData(const std::string& filename, const std::vector<long double>& features)
    : filename_(filename), features_(features) {
    if (features.size() != 66) {
        throw std::invalid_argument("ImageFeatureData requires exactly 66 features: "
            "3 (structural) + 15 (color) + 48 (texture).");
    }
}

int ImageFeatureData::getId() const {
    // 从文件名如 "man_bld_art_n0035.jpg" 中提取数字 ID（如 35）
    size_t pos = filename_.rfind('n');
    if (pos != std::string::npos && pos + 1 < filename_.size()) {
        std::string numStr;
        for (size_t i = pos + 1; i < filename_.size(); ++i) {
            if (std::isdigit(filename_[i])) {
                numStr += filename_[i];
            }
            else {
                break;
            }
        }
        if (!numStr.empty()) {
            try {
                return std::stoi(numStr);
            }
            catch (...) {}
        }
    }
    return -1; // 无法解析时返回 -1
}

std::string ImageFeatureData::toString() const {
    std::string s = filename_;
    for (long double f : features_) {
        s += " " + std::to_string(static_cast<double>(f)); // to_string 不支持 long double
    }
    return s;
}

// ---------- ImageFeatureDistance 实现 ----------
static long double euclideanDistance(const long double* a, const long double* b, size_t n) {
    long double sum = 0.0L;
    for (size_t i = 0; i < n; ++i) {
        long double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

static long double manhattanDistance(const long double* a, const long double* b, size_t n) {
    long double sum = 0.0L;
    for (size_t i = 0; i < n; ++i) {
        sum += std::abs(a[i] - b[i]);
    }
    return sum;
}

long double ImageFeatureDistance::distance(const MetricData& a, const MetricData& b) const {
    const ImageFeatureData* imgA = dynamic_cast<const ImageFeatureData*>(&a);
    const ImageFeatureData* imgB = dynamic_cast<const ImageFeatureData*>(&b);
    if (!imgA || !imgB) {
        throw std::invalid_argument("ImageFeatureDistance only supports ImageFeatureData.");
    }

    // 结构特征：3D，欧氏距离
    long double d_struct = euclideanDistance(imgA->getStructural(), imgB->getStructural(), 3);

    // 颜色特征：15D，曼哈顿距离
    long double d_color = manhattanDistance(imgA->getColor(), imgB->getColor(), 15);

    // 纹理特征：48D，欧氏距离
    long double d_texture = euclideanDistance(imgA->getTexture(), imgB->getTexture(), 48);

    // 线性组合：取平均
    return (d_struct + d_color + d_texture) / 3.0L;
}

std::string ImageFeatureDistance::getName() const {
    return "ImageFeatureDistance(L2_struct + L1_color + L2_texture)/3";
}