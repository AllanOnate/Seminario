#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "atomics/generator.hpp"

// Cadmium v2 headers: support both include layouts.
#include <cadmium/simulation/root_coordinator.hpp>
#include <cadmium/simulation/logger/csv.hpp>

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

class GeneratorTest : public cadmium::Coupled {
  public:
    cadmium::Port<Patient> Out_pacientes;

    GeneratorTest(std::string id, GeneratorConfig cfg)
    : cadmium::Coupled(id)
    {
        Out_pacientes = addOutPort<Patient>("Out_pacientes");

        auto gen = addComponent<GeneratorPacientes>("GeneratorPacientes", cfg);

        addCoupling(gen->Out_pacientes, Out_pacientes);
    }
};

int main() {
    GeneratorConfig cfg;
    cfg.rng_seed = 1;
    cfg.arrivals_csv_path = "input_data/arrivals.csv";
    cfg.max_patients = 0; // ignored in CSV mode

    std::filesystem::create_directories("simulation_results");

    auto model = std::make_shared<GeneratorTest>("GeneratorTest", cfg);
    auto root = cadmium::RootCoordinator(model);

    cesfam_compat::attach_csv_logger(root, "simulation_results/test_generator.csv", ";");

    root.start();
    root.simulate(600.0);
    root.stop();

    std::cout << "Generator test finished.\n";
    return 0;
}
