#pragma once

#include <limits>
#include <random>
#include <deque>
#include <string>
#include <ostream>
#include <utility>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"

namespace cesfam {

struct DoctorConfig {
    int doctor_id = 0;
    unsigned rng_seed = 100;
    double service_mean = 600.0; // seconds (10 minutes)
};

struct DoctorState {
    double now = 0.0;
    bool busy = false;

    std::deque<Patient> queue;
    Patient current;

    double finish_time = std::numeric_limits<double>::infinity(); // absolute time when current service finishes
};

inline std::ostream& operator<<(std::ostream& os, const DoctorState& s) {
    os << "DoctorState{now=" << s.now
       << ", busy=" << (s.busy ? "true" : "false")
       << ", q=" << s.queue.size()
       << ", finish=" << s.finish_time
       << ", current_id=" << s.current.id_paciente
       << "}";
    return os;
}

class Medico : public cadmium::Atomic<DoctorState> {
  public:
    mutable cadmium::Port<Patient> In_Paciente;
    mutable cadmium::Port<Patient> Out_Paciente;

    Medico(std::string id, DoctorConfig cfg)
    : cadmium::Atomic<DoctorState>(id, DoctorState{})
    , cfg_(std::move(cfg))
    , rng_(cfg_.rng_seed)
    , exp_(cfg_.service_mean > 0.0 ? (1.0 / cfg_.service_mean) : 1.0)
    {
        In_Paciente = addInPort<Patient>("In_Paciente");
        Out_Paciente = addOutPort<Patient>("Out_Paciente");
    }

    double timeAdvance(const DoctorState& state) const override {
        if (!state.busy) return std::numeric_limits<double>::infinity();
        const double sigma = state.finish_time - state.now;
        return sigma < 0.0 ? 0.0 : sigma;
    }

    void output(const DoctorState& state) const override {
        if (!state.busy) return;

        Patient p = state.current;
        // The output is produced at finish_time (absolute)
        p.hora_salida = state.finish_time;
        p.tiempo_atencion = p.hora_salida - p.hora_atencion;
        p.estado = PatientStatus::Finalizado; // finished doctor's attention (final decision happens later)
        Out_Paciente->addMessage(p);
    }

    void internalTransition(DoctorState& state) const override {
        if (!state.busy) return;

        // Advance to finish_time
        state.now = state.finish_time;
        state.busy = false;
        state.finish_time = std::numeric_limits<double>::infinity();
        state.current = Patient{};

        // If patients are waiting, start next immediately
        start_if_idle(state);
    }

    void externalTransition(DoctorState& state, double e) const override {
        state.now += e;

        // Enqueue all arrivals
        for (auto p : In_Paciente->getBag()) {
            state.queue.push_back(std::move(p));
        }

        // Start service if possible
        start_if_idle(state);
    }

  private:
    void start_if_idle(DoctorState& state) const {
        if (state.busy) return;
        if (state.queue.empty()) return;

        state.current = std::move(state.queue.front());
        state.queue.pop_front();

        // Assign doctor and mark start of attention
        state.current.medico_asignado = cfg_.doctor_id;
        state.current.hora_atencion = state.now;
        state.current.tiempo_espera = state.current.hora_atencion - state.current.hora_llegada;
        state.current.estado = PatientStatus::EnAtencion;

        const double service = sample_service_time();
        state.current.tiempo_atencion = service;

        state.busy = true;
        state.finish_time = state.now + service;
    }

    double sample_service_time() const {
        if (cfg_.service_mean <= 0.0) return 0.0;
        double s = exp_(rng_);
        if (s <= 0.0) s = std::numeric_limits<double>::min();
        return s;
    }

    DoctorConfig cfg_;
    mutable std::mt19937 rng_;
    mutable std::exponential_distribution<double> exp_;
};

} // namespace cesfam
