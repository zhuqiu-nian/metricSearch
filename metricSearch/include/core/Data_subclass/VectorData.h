#pragma once
#include "../../interfaces/MetricData.h"

// ����������
class VectorData : public MetricData {
public:
    VectorData(vector<long double> vec, int id = -1);
    const vector<long double>& getVector() const;
    int getId() const;
    string toString() const override;

private:
    vector<long double> vector_;
    int id_;
};