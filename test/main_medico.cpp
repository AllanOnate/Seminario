#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "utils/cadmium_includes.hpp"
#include "atomics/generator.hpp"
#include "atomics/doctor.hpp"
#include "data_structures/patient.hpp"

// Cadmium v2 headers: support both include layouts.
#if __has_include(<cadmium/core/simulation/root_coordinator.hpp>)
  #include <cadmium/core/simulation/root_coordinator.hpp>
#elif __has_include(<cadmium/simulation/root_coordinator.hpp>)
  #include <cadmium/simulation/root_coordinator.hpp>
#else
  #error "Cadmium v2 root coordinator header not found. Check CADMIUM_V2_INCLUDE points to .../cadmium_v2/include"
#endif

#if __has_include(<cadmium/core/logger/csv.hpp>)
  #include <cadmium/core/logger/csv.hpp>
#elif __has_include(<cadmium/simulation/logger/csv.hpp>)
  #include <cadmium/simulation/logger/csv.hpp>
#else
  #error "Cadmium v2 CSV logger header not found. Check CADMIUM_V2_INCLUDE points to .../cadmium_v2/include"
#endif

// Logger API compatibility (some variants use setLogger(ptr), others use setLogger<Logger>(...)).
namespace cesfam_compat {
  template <class Root>
  auto attach_csv_logger(Root& root, const std::string& file, const std::string& sep, int)
      -> decltype(root.template setLogger<cadmium::CSVLogger>(file, sep), void()) {
    root.template setLogger<cadmium::CSVLogger>(file, sep);
  }
  template <class Root>
  auto attach_csv_logger(Root& root, const std::string& file, const std::string& sep, long)
      -> decltype(root.setLogger(std::make_shared<cadmium::CSVLogger>(file, sep)), void()) {
    root.setLogger(std::make_shared<cadmium::CSVLogger>(file, sep));
  }
  template <class Root>
  void attach_csv_logger(Root& root, const std::string& file, const std::string& sep) {
    attach_csv_logger(root, file, sep, 0);
  }
}  // namespace cesfam_compat

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

    cesfam_compat::attach_csv_logger(root, "simulation_results/test_medico.csv", ";");

    root.start();
    root.simulate(1200.0);
    root.stop();

    std::cout << "Medico test finished.\n";
    return 0;
}
