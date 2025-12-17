#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "atomics/generator.hpp"

#if __has_include(<cadmium/core/simulation/root_coordinator.hpp>)
  #include <cadmium/core/simulation/root_coordinator.hpp>
#endif
#if __has_include(<cadmium/core/logger/csv.hpp>)
  #include <cadmium/core/logger/csv.hpp>
#endif

using namespace cesfam;

int main() {
    GeneratorConfig cfg;
    cfg.rng_seed = 1;
    cfg.arrivals_csv_path = "input_data/arrivals.csv";
    cfg.max_patients = 0; // ignored in CSV mode

    std::filesystem::create_directories("simulation_results");

    auto model = std::make_shared<GeneratorPacientes>("GeneratorPacientes", cfg);
    auto root = cadmium::RootCoordinator(model);

    auto logger = std::make_shared<cadmium::CSVLogger>("simulation_results/test_generator.csv", ";");
    root.setLogger(logger);

    root.start();
    root.simulate(600.0);
    root.stop();

    std::cout << "Generator test finished.\n";
    return 0;
}
