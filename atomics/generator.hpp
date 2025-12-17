#pragma once

#include <limits>
#include <random>
#include <vector>
#include <string>
#include <ostream>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"
#include "utils/csv_arrivals.hpp"

namespace cesfam {

struct GeneratorConfig {
    unsigned rng_seed = 1;
    double arrivals_rate = 0.1;            // patients per second (if using stochastic mode)
    std::string arrivals_csv_path = "";    // if non-empty, use deterministic schedule
    int max_patients = 100;                // 0 = unlimited (only for stochastic)
    int default_age = 70;
};

struct GeneratorState {
    double now = 0.0;          // current simulation time
    double next = 0.0;         // next generation absolute time
    int next_id = 1;
    std::size_t schedule_idx = 0;
    bool done = false;
};

inline std::ostream& operator<<(std::ostream& os, const GeneratorState& s) {
    os << "GeneratorState{now=" << s.now << ", next=" << s.next
       << ", next_id=" << s.next_id << ", idx=" << s.schedule_idx
       << ", done=" << (s.done ? "true" : "false") << "}";
    return os;
}

class GeneratorPacientes : public cadmium::Atomic<GeneratorState> {
  public:
    cadmium::Port<Patient> Out_pacientes;

    GeneratorPacientes(std::string id, GeneratorConfig cfg)
    : cadmium::Atomic<GeneratorState>(id, initial_state(cfg))
    , cfg_(std::move(cfg))
    , rng_(cfg_.rng_seed)
    , exp_(cfg_.arrivals_rate > 0.0 ? cfg_.arrivals_rate : 1.0)   // lambda
    {
        Out_pacientes = addOutPort<Patient>("Out_pacientes");

        if (!cfg_.arrivals_csv_path.empty()) {
            schedule_ = read_arrivals_csv(cfg_.arrivals_csv_path);
        }
    }

    double timeAdvance(const GeneratorState& state) const override {
        if (state.done) return std::numeric_limits<double>::infinity();
        return std::max(0.0, state.next - state.now);
    }

    void output(GeneratorState& state) const override {
        if (state.done) return;

        Patient p;
        p.id_paciente = state.next_id;
        p.estado = PatientStatus::Generado;
        p.hora_llegada = state.next;

        if (!schedule_.empty() && state.schedule_idx < schedule_.size()) {
            const auto& ev = schedule_[state.schedule_idx];
            p.edad = ev.edad > 0 ? ev.edad : cfg_.default_age;
            p.nivel_riesgo = ev.riesgo;
        } else {
            p.edad = cfg_.default_age; // could be randomized later
            p.nivel_riesgo = RiskLevel::Unknown;
        }

        Out_pacientes.addMessage(p);
    }

    void internalTransition(GeneratorState& state) const override {
        if (state.done) return;

        // advance time to the internal event instant
        state.now = state.next;

        // prepare next event
        state.next_id += 1;

        if (!schedule_.empty()) {
            state.schedule_idx += 1;
            if (state.schedule_idx >= schedule_.size()) {
                state.done = true;
                state.next = std::numeric_limits<double>::infinity();
            } else {
                state.next = schedule_[state.schedule_idx].time;
            }
            return;
        }

        // stochastic mode
        if (cfg_.max_patients > 0 && state.next_id > cfg_.max_patients) {
            state.done = true;
            state.next = std::numeric_limits<double>::infinity();
            return;
        }
        if (cfg_.arrivals_rate <= 0.0) {
            state.done = true;
            state.next = std::numeric_limits<double>::infinity();
            return;
        }

        double delta = exp_(rng_);
        if (delta <= 0.0) delta = std::numeric_limits<double>::min();
        state.next = state.now + delta;
    }

    void externalTransition(GeneratorState& state, double e) const override {
        // generator has no inputs, but for completeness we advance the internal clock
        state.now += e;
    }

  private:
    static GeneratorState initial_state(const GeneratorConfig& cfg) {
    GeneratorState s;
    s.now = 0.0;
    s.next_id = 1;
    s.schedule_idx = 0;
    s.done = false;

    if (!cfg.arrivals_csv_path.empty()) {
        // Deterministic schedule mode
        auto sch = read_arrivals_csv(cfg.arrivals_csv_path);
        if (sch.empty()) {
            s.done = true;
            s.next = std::numeric_limits<double>::infinity();
        } else {
            s.next = sch.front().time;
        }
        return s;
    }

    // Stochastic mode
    if (cfg.arrivals_rate > 0.0) {
        // Use mean inter-arrival as initial guess.
        s.next = 1.0 / cfg.arrivals_rate;
    } else {
        s.done = true;
        s.next = std::numeric_limits<double>::infinity();
    }
    return s;
}

GeneratorConfig cfg_;

    mutable std::mt19937 rng_;
    mutable std::exponential_distribution<double> exp_;
    std::vector<ArrivalEvent> schedule_;
};

} // namespace cesfam
