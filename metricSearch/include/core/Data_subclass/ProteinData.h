#pragma once
#pragma once
#include "../../interfaces/MetricData.h"
#include <string>

class ProteinData : public MetricData {
    std::string sequence_;
    int id_;

public:
    ProteinData(const std::string& seq, int id)
        : sequence_(seq), id_(id) {}

    const std::string& getSequence() const { return sequence_; }
    int getId() const override { return id_; }
    std::string toString() const override {
        return "ProteinData(id=" + std::to_string(id_) + ", seq=" + sequence_ + ")";
    }
};
