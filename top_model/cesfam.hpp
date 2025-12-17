#pragma once

#include <string>
#include <memory>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"

#include "atomics/generator.hpp"
#include "atomics/case_manager.hpp"
#include "atomics/adherence.hpp"
#include "coupled/medical_staff.hpp"

namespace cesfam {

struct CesfamConfig {
    GeneratorConfig generator;
    CaseManagerConfig case_manager;
    MedicalStaffConfig medical_staff;
    AdherenceConfig adherence;
};

class CESFAM : public cadmium::Coupled {
  public:
    cadmium::Port<Patient> Out_PacienteRA;
    cadmium::Port<Patient> Out_PacienteRC;

    explicit CESFAM(std::string id, CesfamConfig cfg)
    : cadmium::Coupled(id)
    , cfg_(std::move(cfg))
    {
        Out_PacienteRA = addOutPort<Patient>("Out_PacienteRA");
        Out_PacienteRC = addOutPort<Patient>("Out_PacienteRC");

        auto gen = addComponent<GeneratorPacientes>("GeneradorPacientes", cfg_.generator);
        auto gestor = addComponent<GestorCasos>("GestorCasos", cfg_.case_manager);
        auto equipo = addComponent<EquipoMedico>("EquipoMedico", cfg_.medical_staff);
        auto adher = addComponent<AdherenciaDecision>("AdherenciaDecision", cfg_.adherence);

        // Generator -> Case manager
        addCoupling(gen->Out_pacientes, gestor->In_paciente);

        // Case manager -> Medical staff
        addCoupling(gestor->Out_pacienteAC, equipo->In_Paciente);

        // Medical staff -> Adherence/decision
        addCoupling(equipo->Out_Paciente, adher->In_Paciente);

        // Adherence return loop -> Case manager
        addCoupling(adher->Out_PacienteDA, gestor->In_PacienteDA);

        // Final exits
        addCoupling(adher->Out_PacienteRA, Out_PacienteRA);
        addCoupling(gestor->Out_PacienteRC, Out_PacienteRC);
    }

  private:
    CesfamConfig cfg_;
};

} // namespace cesfam
