#include "../../../include/core/Data_subclass/VectorData.h"

// VectorData 实现
VectorData::VectorData(vector<long double> vec, int id) : vector_(move(vec)), id_(id) {}

const vector<long double>& VectorData::getVector() const { return vector_; }

int VectorData::getId() const { return id_; }

string VectorData::toString() const {
    string str = "向量数据(id=" + to_string(id_) + ", 向量=[";
    for (size_t i = 0; i < vector_.size(); ++i) {
        if (i != 0) str += ", ";
        str += to_string(vector_[i]);
    }
    str += "])";
    return str;
}