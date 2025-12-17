#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include "data_structures/patient.hpp"
#include "utils/ini_reader.hpp"

namespace cesfam {

struct ArrivalEvent {
    double time = 0.0;
    int edad = 0;
    RiskLevel riesgo = RiskLevel::Unknown;
};

inline RiskLevel parse_risk(std::string s) {
    s = trim_copy(s);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    if (s == "alto" || s == "high") return RiskLevel::Alto;
    if (s == "medio" || s == "medium") return RiskLevel::Medio;
    if (s == "bajo" || s == "low") return RiskLevel::Bajo;
    return RiskLevel::Unknown;
}

inline std::vector<ArrivalEvent> read_arrivals_csv(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open arrivals CSV: " + path);
    }
    std::vector<ArrivalEvent> out;
    std::string line;
    bool first = true;

    while (std::getline(in, line)) {
        line = trim_copy(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        // Skip header if present
        if (first) {
            first = false;
            if (line.find("time") != std::string::npos || line.find("hora") != std::string::npos) {
                continue;
            }
        }

        std::stringstream ss(line);
        std::string t, edad, riesgo;
        std::getline(ss, t, ',');
        std::getline(ss, edad, ',');
        std::getline(ss, riesgo, ',');

        ArrivalEvent ev;
        ev.time = std::stod(trim_copy(t));
        ev.edad = edad.empty() ? 0 : std::stoi(trim_copy(edad));
        ev.riesgo = riesgo.empty() ? RiskLevel::Unknown : parse_risk(riesgo);

        out.push_back(ev);
    }

    std::sort(out.begin(), out.end(), [](const ArrivalEvent& a, const ArrivalEvent& b){ return a.time < b.time; });
    return out;
}

} // namespace cesfam
