#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <string>
#include <fstream>
#include <limits>
#include <algorithm>
#include <chrono>
#include "MetricData.h"

using namespace std;

// 度量空间距离函数父类
class MetricDistance {
public:
    virtual ~MetricDistance() = default;
    virtual long double distance(const MetricData& a, const MetricData& b) const = 0;
    virtual string getName() const = 0;
};
