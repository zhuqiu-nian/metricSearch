#pragma once
#include "../../interfaces/MetricData.h"
#include <string>

class StringData : public MetricData {
    std::string str_;
    int id_;
public:
    StringData(const std::string& s, int id) : str_(s), id_(id) {}

    const std::string& getString() const { return str_; }
    int getId() const  { return id_; }
    std::string toString() const override {
        return "StringData(id=" + std::to_string(id_) + ", str=" + str_ + ")";
    }
};
