#pragma once

#include <limits>
#include <vector>
#include <string>
#include <ostream>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"

namespace cesfam {

struct RouterConfig {
    int doctors = 3;
};

struct RouterState {
    double now = 0.0;
    int rr_index = 0;  // round-robin pointer
    std::vector<std::vector<Patient>> out_to_doc;
};

inline std::ostream& operator<<(std::ostream& os, const RouterState& s) {
    std::size_t pending = 0;
    for (const auto& v : s.out_to_doc) pending += v.size();
    os << "RouterState{now=" << s.now << ", rr=" << s.rr_index
       << ", pending=" << pending << "}";
    return os;
}

class RouterMedicos : public cadmium::Atomic<RouterState> {
  public:
    mutable cadmium::Port<Patient> In_paciente;
    mutable std::vector<cadmium::Port<Patient>> Out_to_doctor;

    RouterMedicos(std::string id, RouterConfig cfg)
    : cadmium::Atomic<RouterState>(id, initial_state(cfg))
    , cfg_(std::move(cfg))
    {
        if (cfg_.doctors <= 0) cfg_.doctors = 1;

        In_paciente = addInPort<Patient>("In_paciente");

        Out_to_doctor.reserve(cfg_.doctors);
        for (int i = 0; i < cfg_.doctors; ++i) {
            Out_to_doctor.push_back(addOutPort<Patient>("Out_doctor_" + std::to_string(i)));
        }
    }

    double timeAdvance(const RouterState& state) const override {
        std::size_t pending = 0;
        for (const auto& v : state.out_to_doc) pending += v.size();
        if (pending > 0) return 0.0;
        return std::numeric_limits<double>::infinity();
    }

    void output(const RouterState& state) const override {
        for (int i = 0; i < static_cast<int>(state.out_to_doc.size()); ++i) {
            for (const auto& p : state.out_to_doc[i]) {
                Out_to_doctor[i]->addMessage(p);
            }
        }
    }

    void internalTransition(RouterState& state) const override {
        for (auto& v : state.out_to_doc) v.clear();
    }

    void externalTransition(RouterState& state, double e) const override {
        state.now += e;

        for (auto p : In_paciente->getBag()) {
            int idx = state.rr_index % cfg_.doctors;
            state.out_to_doc[idx].push_back(std::move(p));
            state.rr_index = (state.rr_index + 1) % cfg_.doctors;
        }
    }

  private:
    static RouterState initial_state(const RouterConfig& cfg) {
        RouterState s;
        s.now = 0.0;
        s.rr_index = 0;
        int n = cfg.doctors <= 0 ? 1 : cfg.doctors;
        s.out_to_doc.assign(static_cast<std::size_t>(n), {});
        return s;
    }

    RouterConfig cfg_;
};

} // namespace cesfam
