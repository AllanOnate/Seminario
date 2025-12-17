#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "utils/cadmium_includes.hpp"
#include "atomics/generator.hpp"
#include "atomics/doctor.hpp"
#include "data_structures/patient.hpp"

#if __has_include(<cadmium/core/simulation/root_coordinator.hpp>)
  #include <cadmium/core/simulation/root_coordinator.hpp>
#endif
#if __has_include(<cadmium/core/logger/csv.hpp>)
  #include <cadmium/core/logger/csv.hpp>
#endif

using namespace cesfam;

class MedicoTest : public cadmium::Coupled {
  public:
    cadmium::Port<Patient> Out_Paciente;

    MedicoTest(std::string id)
    : cadmium::Coupled(id)
    {
        Out_Paciente = addOutPort<Patient>("Out_Paciente");

        GeneratorConfig gcfg;
        gcfg.arrivals_csv_path = "input_data/arrivals.csv";
        gcfg.rng_seed = 1;

        DoctorConfig dcfg;
        dcfg.doctor_id = 0;
        dcfg.rng_seed = 123;
        dcfg.service_mean = 120.0; // faster for test

        auto gen = addComponent<GeneratorPacientes>("GeneratorPacientes", gcfg);
        auto doc = addComponent<Medico>("Medico", dcfg);

        addCoupling(gen->Out_pacientes, doc->In_Paciente);
        addCoupling(doc->Out_Paciente, Out_Paciente);
    }
};

int main() {
    std::filesystem::create_directories("simulation_results");

    auto model = std::make_shared<MedicoTest>("MedicoTest");
    auto root = cadmium::RootCoordinator(model);

    auto logger = std::make_shared<cadmium::CSVLogger>("simulation_results/test_medico.csv", ";");
    root.setLogger(logger);

    root.start();
    root.simulate(1200.0);
    root.stop();

    std::cout << "Medico test finished.\n";
    return 0;
}
