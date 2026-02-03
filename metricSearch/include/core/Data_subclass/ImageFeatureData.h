// ImageFeatureData.h
#pragma once
#include "../../interfaces/MetricData.h"
#include <vector>
#include <string>

class ImageFeatureData : public MetricData {
private:
    std::string filename_;
    std::vector<long double> features_; // 长度必须为 66

public:
    ImageFeatureData(const std::string& filename, const std::vector<long double>& features);

    const std::string& getFilename() const { return filename_; }
    const std::vector<long double>& getFeatures() const { return features_; }

    // 获取各特征子向量的起始指针（高效，避免复制）
    const long double* getStructural() const { return features_.data(); }     // 3D
    const long double* getColor() const { return features_.data() + 3; }      // 15D
    const long double* getTexture() const { return features_.data() + 18; }   // 48D

    int getId() const override;
    std::string toString() const override;
};