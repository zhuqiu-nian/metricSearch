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

using namespace std;

// 度量空间数据父类
class MetricData {
public:
    virtual ~MetricData() = default;
    virtual string toString() const = 0;
    virtual int getId() const = 0;
};
