#pragma once

#include <ostream>
#include <string>
#include <cstdint>

namespace cesfam {

enum class RiskLevel : std::uint8_t { Unknown=0, Alto, Medio, Bajo };
enum class PatientStatus : std::uint8_t {
    Unknown=0,
    Generado,
    EsperandoEvaluacion,
    EvaluadoPriorizado,
    EnEsperaAtencion,
    EnAtencion,
    Finalizado,
    Derivado
};
enum class AttentionResult : std::uint8_t { Unknown=0, Alta, Derivacion, Seguimiento };

inline const char* to_string(RiskLevel r) {
    switch (r) {
        case RiskLevel::Alto: return "alto";
        case RiskLevel::Medio: return "medio";
        case RiskLevel::Bajo: return "bajo";
        default: return "unknown";
    }
}
inline const char* to_string(PatientStatus s) {
    switch (s) {
        case PatientStatus::Generado: return "generado";
        case PatientStatus::EsperandoEvaluacion: return "esperando_evaluacion";
        case PatientStatus::EvaluadoPriorizado: return "evaluado_priorizado";
        case PatientStatus::EnEsperaAtencion: return "en_espera_atencion";
        case PatientStatus::EnAtencion: return "en_atencion";
        case PatientStatus::Finalizado: return "finalizado";
        case PatientStatus::Derivado: return "derivado";
        default: return "unknown";
    }
}
inline const char* to_string(AttentionResult r) {
    switch (r) {
        case AttentionResult::Alta: return "alta";
        case AttentionResult::Derivacion: return "derivacion";
        case AttentionResult::Seguimiento: return "seguimiento";
        default: return "unknown";
    }
}

/// Message that travels through the DEVS network (Patient).
/// Fields align with the thesis "Diccionario de datos del paciente simulado"
/// (id_paciente, nivel_riesgo, estado, hora_llegada, hora_atencion, tiempo_espera,
/// tiempo_atencion, medico_asignado, resultado). Additional fields are included
/// for simulation control (e.g., followups_done).
struct Patient {
    int id_paciente = -1;

    // Domain / triage
    int edad = 0;
    RiskLevel nivel_riesgo = RiskLevel::Unknown;

    // DEVS lifecycle
    PatientStatus estado = PatientStatus::Unknown;

    // Times are "simulation seconds since t0"
    double hora_llegada = 0.0;   // arrival time
    double hora_atencion = 0.0;  // time when medical attention starts
    double hora_salida = 0.0;    // (extra) time when attention ends / leaves doctor

    // Durations in seconds
    double tiempo_espera = 0.0;
    double tiempo_atencion = 0.0;

    int medico_asignado = -1;
    AttentionResult resultado = AttentionResult::Unknown;

    // (extra) how many times the patient has returned for follow-up (DA loop)
    int followups_done = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Patient& p) {
    os << "Patient{id=" << p.id_paciente
       << ", edad=" << p.edad
       << ", riesgo=" << to_string(p.nivel_riesgo)
       << ", estado=" << to_string(p.estado)
       << ", llegada=" << p.hora_llegada
       << ", atencion=" << p.hora_atencion
       << ", salida=" << p.hora_salida
       << ", espera_s=" << p.tiempo_espera
       << ", atencion_s=" << p.tiempo_atencion
       << ", medico=" << p.medico_asignado
       << ", resultado=" << to_string(p.resultado)
       << ", followups=" << p.followups_done
       << "}";
    return os;
}

} // namespace cesfam
