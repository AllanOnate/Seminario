#pragma once

#include <memory>
#include <vector>
#include <string>

#include "utils/cadmium_includes.hpp"
#include "data_structures/patient.hpp"
#include "atomics/router.hpp"
#include "atomics/doctor.hpp"

namespace cesfam {

struct MedicalStaffConfig {
    int doctors = 3;
    double service_mean = 600.0;       // seconds
    unsigned rng_seed_base = 1000;     // base seed for doctors
};

class EquipoMedico : public cadmium::Coupled {
  public:
    cadmium::Port<Patient> In_Paciente;
    cadmium::Port<Patient> Out_Paciente;

    EquipoMedico(std::string id, MedicalStaffConfig cfg)
    : cadmium::Coupled(id)
    , cfg_(std::move(cfg))
    {
        if (cfg_.doctors <= 0) cfg_.doctors = 1;

        In_Paciente = addInPort<Patient>("In_Paciente");
        Out_Paciente = addOutPort<Patient>("Out_Paciente");

        // Router
        auto router = addComponent<RouterMedicos>("RouterMedicos", RouterConfig{cfg_.doctors});

        // Doctors
        doctors_.reserve(cfg_.doctors);
        for (int i = 0; i < cfg_.doctors; ++i) {
            DoctorConfig dc;
            dc.doctor_id = i;
            dc.rng_seed = cfg_.rng_seed_base + static_cast<unsigned>(i);
            dc.service_mean = cfg_.service_mean;

            auto doc = addComponent<Medico>("Medico_" + std::to_string(i), dc);
            doctors_.push_back(doc);

            // IC: router -> doctor
            addCoupling(router->Out_to_doctor[i], doc->In_Paciente);

            // EOC: doctor -> out
            addCoupling(doc->Out_Paciente, Out_Paciente);
        }

        // EIC: in -> router
        addCoupling(In_Paciente, router->In_paciente);
    }

  private:
    MedicalStaffConfig cfg_;
    std::vector<std::shared_ptr<Medico>> doctors_;
};

} // namespace cesfam
