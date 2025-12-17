#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "utils/cadmium_includes.hpp"
#include "atomics/generator.hpp"
#include "atomics/case_manager.hpp"
#include "data_structures/patient.hpp"

#if __has_include(<cadmium/core/simulation/root_coordinator.hpp>)
  #include <cadmium/core/simulation/root_coordinator.hpp>
#endif
#if __has_include(<cadmium/core/logger/csv.hpp>)
  #include <cadmium/core/logger/csv.hpp>
#endif

using namespace cesfam;

class GestorTest : public cadmium::Coupled {
  public:
    cadmium::Port<Patient> Out_AC;
    cadmium::Port<Patient> Out_RC;

    GestorTest(std::string id)
    : cadmium::Coupled(id)
    {
        Out_AC = addOutPort<Patient>("Out_AC");
        Out_RC = addOutPort<Patient>("Out_RC");

        GeneratorConfig gcfg;
        gcfg.arrivals_csv_path = "input_data/arrivals.csv";
        gcfg.rng_seed = 1;

        CaseManagerConfig ccfg;
        ccfg.rng_seed = 2;
        ccfg.consent_p_accept = 0.8;

        auto gen = addComponent<GeneratorPacientes>("GeneratorPacientes", gcfg);
        auto gestor = addComponent<GestorCasos>("GestorCasos", ccfg);

        addCoupling(gen->Out_pacientes, gestor->In_paciente);
        addCoupling(gestor->Out_pacienteAC, Out_AC);
        addCoupling(gestor->Out_PacienteRC, Out_RC);
    }
};

int main() {
    std::filesystem::create_directories("simulation_results");

    auto model = std::make_shared<GestorTest>("GestorTest");
    auto root = cadmium::RootCoordinator(model);

    auto logger = std::make_shared<cadmium::CSVLogger>("simulation_results/test_gestor.csv", ";");
    root.setLogger(logger);

    root.start();
    root.simulate(600.0);
    root.stop();

    std::cout << "Gestor test finished.\n";
    return 0;
}
