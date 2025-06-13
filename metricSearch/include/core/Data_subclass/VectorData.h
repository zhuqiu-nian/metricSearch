#pragma once
#include "../../interfaces/MetricData.h"

// ����������
class VectorData : public MetricData {
public:
    VectorData(vector<double> vec, int id = -1);
    const vector<double>& getVector() const;
    int getId() const;
    string toString() const override;

private:
    vector<double> vector_;
    int id_;
};