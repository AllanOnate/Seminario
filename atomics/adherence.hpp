#pragma once

#include <limits>
#include <random>
#include <vector>
#include <string>
#include <ostream>
#include <algorithm>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"

namespace cesfam {

struct AdherenceConfig {
    unsigned rng_seed = 3;

    // Base probability of continuing / returning for follow-up (DA)
    double p_continue_base = 0.30;

    // Multipliers by risk group (p = clamp(base * mult, 0..1))
    double mult_alto = 1.20;
    double mult_medio = 1.00;
    double mult_bajo = 0.80;

    int max_followups = 3; // prevents infinite loops
};

struct AdherenceState {
    double now = 0.0;
    std::vector<Patient> out_DA;
    std::vector<Patient> out_RA;
};

inline std::ostream& operator<<(std::ostream& os, const AdherenceState& s) {
    os << "AdherenceState{now=" << s.now
       << ", out_DA=" << s.out_DA.size()
       << ", out_RA=" << s.out_RA.size()
       << "}";
    return os;
}

class AdherenciaDecision : public cadmium::Atomic<AdherenceState> {
  public:
    cadmium::Port<Patient> In_Paciente;
    cadmium::Port<Patient> Out_PacienteDA;
    cadmium::Port<Patient> Out_PacienteRA;

    AdherenciaDecision(std::string id, AdherenceConfig cfg)
    : cadmium::Atomic<AdherenceState>(id, AdherenceState{})
    , cfg_(std::move(cfg))
    , rng_(cfg_.rng_seed)
    , uni01_(0.0, 1.0)
    {
        In_Paciente = addInPort<Patient>("In_Paciente");
        Out_PacienteDA = addOutPort<Patient>("Out_PacienteDA");
        Out_PacienteRA = addOutPort<Patient>("Out_PacienteRA");
    }

    double timeAdvance(const AdherenceState& state) const override {
        if (!state.out_DA.empty() || !state.out_RA.empty()) return 0.0;
        return std::numeric_limits<double>::infinity();
    }

    void output(AdherenceState& state) const override {
        for (const auto& p : state.out_DA) Out_PacienteDA.addMessage(p);
        for (const auto& p : state.out_RA) Out_PacienteRA.addMessage(p);
    }

    void internalTransition(AdherenceState& state) const override {
        state.out_DA.clear();
        state.out_RA.clear();
    }

    void externalTransition(AdherenceState& state, double e) const override {
        state.now += e;

        for (auto p : In_Paciente.getBag()) {
            // Decide if patient returns to the system (DA loop) or exits (RA)
            if (p.followups_done >= cfg_.max_followups) {
                finalize_patient(p);
                state.out_RA.push_back(std::move(p));
                continue;
            }

            double p_return = cfg_.p_continue_base;
            switch (p.nivel_riesgo) {
                case RiskLevel::Alto:  p_return *= cfg_.mult_alto; break;
                case RiskLevel::Medio: p_return *= cfg_.mult_medio; break;
                case RiskLevel::Bajo:  p_return *= cfg_.mult_bajo; break;
                default: break;
            }
            p_return = std::clamp(p_return, 0.0, 1.0);

            const double u = uni01_(rng_);
            const bool returns = (u <= p_return);

            if (returns) {
                p.followups_done += 1;
                p.resultado = AttentionResult::Seguimiento;
                // state of patient will be reset by the CaseManager upon re-entry
                state.out_DA.push_back(std::move(p));
            } else {
                finalize_patient(p);
                state.out_RA.push_back(std::move(p));
            }
        }
    }

  private:
    void finalize_patient(Patient& p) const {
        // Simple policy: most patients discharged; some are derived.
        double p_deriv = 0.05;
        if (p.nivel_riesgo == RiskLevel::Alto) p_deriv = 0.15;

        const double u = uni01_(rng_);
        if (u <= p_deriv) {
            p.estado = PatientStatus::Derivado;
            p.resultado = AttentionResult::Derivacion;
        } else {
            p.estado = PatientStatus::Finalizado;
            p.resultado = AttentionResult::Alta;
        }
    }

    AdherenceConfig cfg_;
    mutable std::mt19937 rng_;
    mutable std::uniform_real_distribution<double> uni01_;
};

} // namespace cesfam
