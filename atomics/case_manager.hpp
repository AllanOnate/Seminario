#pragma once

#include <limits>
#include <random>
#include <vector>
#include <string>
#include <ostream>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"

namespace cesfam {

struct CaseManagerConfig {
    unsigned rng_seed = 2;
    double consent_p_accept = 1.0;   // probability that patient is accepted into the APS process
};

struct CaseManagerState {
    double now = 0.0;

    // Pending outputs (emitted at sigma=0)
    std::vector<Patient> out_AC;
    std::vector<Patient> out_RC;
};

inline std::ostream& operator<<(std::ostream& os, const CaseManagerState& s) {
    os << "CaseManagerState{now=" << s.now
       << ", out_AC=" << s.out_AC.size()
       << ", out_RC=" << s.out_RC.size()
       << "}";
    return os;
}

class GestorCasos : public cadmium::Atomic<CaseManagerState> {
  public:
    // Inputs
    mutable cadmium::Port<Patient> In_paciente;
    mutable cadmium::Port<Patient> In_PacienteDA;

    // Outputs
    mutable cadmium::Port<Patient> Out_pacienteAC;
    mutable cadmium::Port<Patient> Out_PacienteRC;

    GestorCasos(std::string id, CaseManagerConfig cfg)
    : cadmium::Atomic<CaseManagerState>(id, CaseManagerState{})
    , cfg_(std::move(cfg))
    , rng_(cfg_.rng_seed)
    , uni01_(0.0, 1.0)
    {
        In_paciente = addInPort<Patient>("In_paciente");
        In_PacienteDA = addInPort<Patient>("In_PacienteDA");

        Out_pacienteAC = addOutPort<Patient>("Out_pacienteAC");
        Out_PacienteRC = addOutPort<Patient>("Out_PacienteRC");
    }

    double timeAdvance(const CaseManagerState& state) const override {
        if (!state.out_AC.empty() || !state.out_RC.empty()) return 0.0;
        return std::numeric_limits<double>::infinity();
    }

    void output(const CaseManagerState& state) const override {
        for (const auto& p : state.out_AC) Out_pacienteAC->addMessage(p);
        for (const auto& p : state.out_RC) Out_PacienteRC->addMessage(p);
    }

    void internalTransition(CaseManagerState& state) const override {
        // clear pending outputs
        state.out_AC.clear();
        state.out_RC.clear();
        // note: now does not change because sigma=0
    }

    void externalTransition(CaseManagerState& state, double e) const override {
        state.now += e;

        // New arrivals from generator
        for (auto p : In_paciente->getBag()) {
            p.estado = PatientStatus::EsperandoEvaluacion;
            // keep arrival time set by generator
            handle_patient(state, std::move(p));
        }

        // Returns from adherence/decision module
        for (auto p : In_PacienteDA->getBag()) {
            // treat return as a new arrival into the APS process
            p.hora_llegada = state.now;
            p.hora_atencion = 0.0;
            p.hora_salida = 0.0;
            p.tiempo_espera = 0.0;
            p.tiempo_atencion = 0.0;
            p.medico_asignado = -1;
            p.resultado = AttentionResult::Unknown;
            p.estado = PatientStatus::EsperandoEvaluacion;

            handle_patient(state, std::move(p));
        }
    }

  private:
    void handle_patient(CaseManagerState& state, Patient p) const {
        // Apply Buzón APS classification if not already known.
        if (p.nivel_riesgo == RiskLevel::Unknown) {
            // Simple heuristic: older -> higher risk.
            if (p.edad >= 80) p.nivel_riesgo = RiskLevel::Alto;
            else if (p.edad >= 70) p.nivel_riesgo = RiskLevel::Medio;
            else p.nivel_riesgo = RiskLevel::Bajo;
        }

        p.estado = PatientStatus::EvaluadoPriorizado;

        // Consent/acceptance gate: if not accepted -> RC (exit)
        const double u = uni01_(rng_);
        const bool accepted = (u <= cfg_.consent_p_accept);

        if (!accepted) {
            p.estado = PatientStatus::Finalizado;
            p.resultado = AttentionResult::Derivacion; // treat as "not handled here"
            state.out_RC.push_back(std::move(p));
            return;
        }

        // Accepted: send to medical staff / appointment (AC)
        p.estado = PatientStatus::EnEsperaAtencion;
        state.out_AC.push_back(std::move(p));
    }

    CaseManagerConfig cfg_;
    mutable std::mt19937 rng_;
    mutable std::uniform_real_distribution<double> uni01_;
};

} // namespace cesfam
