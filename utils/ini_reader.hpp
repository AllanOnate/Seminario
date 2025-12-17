#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace cesfam {

inline std::string trim_copy(std::string s) {
    auto notSpace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

inline std::unordered_map<std::string, std::string> read_kv_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    std::unordered_map<std::string, std::string> kv;
    std::string line;
    while (std::getline(in, line)) {
        line = trim_copy(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == ';') continue;

        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim_copy(line.substr(0, pos));
        std::string val = trim_copy(line.substr(pos + 1));
        if (!key.empty()) kv[key] = val;
    }
    return kv;
}

inline bool has_key(const std::unordered_map<std::string, std::string>& kv, const std::string& key) {
    return kv.find(key) != kv.end();
}

inline std::string get_string(const std::unordered_map<std::string, std::string>& kv, const std::string& key, const std::string& def = "") {
    auto it = kv.find(key);
    return it == kv.end() ? def : it->second;
}

inline int get_int(const std::unordered_map<std::string, std::string>& kv, const std::string& key, int def = 0) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    return std::stoi(it->second);
}

inline double get_double(const std::unordered_map<std::string, std::string>& kv, const std::string& key, double def = 0.0) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    return std::stod(it->second);
}

inline bool get_bool(const std::unordered_map<std::string, std::string>& kv, const std::string& key, bool def = false) {
    auto it = kv.find(key);
    if (it == kv.end()) return def;
    auto v = it->second;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c){ return std::tolower(c); });
    return (v == "1" || v == "true" || v == "yes" || v == "on");
}

} // namespace cesfam
