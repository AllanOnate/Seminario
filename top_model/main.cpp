#include <iostream>
#include <limits>
#include <memory>
#include <filesystem>

#include "utils/ini_reader.hpp"
#include "top_model/cesfam.hpp"

// Cadmium v2 simulation engine + logger
//
// Similar to modeling headers, Cadmium v2 can appear with:
//   A) cadmium/core/simulation/... and cadmium/core/logger/...
//   B) cadmium/simulation/... (logger lives under simulation/logger)
//
// We support both layouts.
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

// Some Cadmium v2 variants expose setLogger(logger_ptr), others expose
// setLogger<LoggerType>(args...). We provide a tiny SFINAE-based helper.
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

int main(int argc, char** argv) {
    std::string params_path = "input_data/params.ini";
    if (argc >= 2) params_path = argv[1];

    std::unordered_map<std::string, std::string> kv;
    try {
        kv = read_kv_file(params_path);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        std::cerr << "Usage: " << argv[0] << " [path/to/params.ini]\n";
        return 1;
    }

    // ---- Build config -------------------------------------------------------
    const int global_seed = get_int(kv, "simulation.rng_seed", 1);

    CesfamConfig cfg;

    // Generator
    cfg.generator.rng_seed = static_cast<unsigned>(get_int(kv, "generator.rng_seed", global_seed));
    cfg.generator.arrivals_rate = get_double(kv, "arrivals.rate", 0.05);
    cfg.generator.arrivals_csv_path = get_string(kv, "arrivals.csv", "");
    cfg.generator.max_patients = get_int(kv, "arrivals.max_patients", 100);
    cfg.generator.default_age = get_int(kv, "patients.default_age", 70);

    // Case manager
    cfg.case_manager.rng_seed = static_cast<unsigned>(get_int(kv, "case_manager.rng_seed", global_seed + 1));
    cfg.case_manager.consent_p_accept = get_double(kv, "consent.p_accept", 1.0);

    // Medical staff
    cfg.medical_staff.doctors = get_int(kv, "router.doctors", 3);
    cfg.medical_staff.service_mean = get_double(kv, "service.mean", 600.0);
    cfg.medical_staff.rng_seed_base = static_cast<unsigned>(get_int(kv, "service.rng_seed_base", 1000));

    // Adherence / decision
    cfg.adherence.rng_seed = static_cast<unsigned>(get_int(kv, "adherence.rng_seed", global_seed + 2));
    cfg.adherence.p_continue_base = get_double(kv, "adherence.p_continue_base", 0.30);
    cfg.adherence.mult_alto = get_double(kv, "adherence.mult_alto", 1.20);
    cfg.adherence.mult_medio = get_double(kv, "adherence.mult_medio", 1.00);
    cfg.adherence.mult_bajo = get_double(kv, "adherence.mult_bajo", 0.80);
    cfg.adherence.max_followups = get_int(kv, "adherence.max_followups", 3);

    // ---- Simulation params --------------------------------------------------
    const double until = get_double(kv, "simulation.until", 3600.0);
    const std::string out_csv = get_string(kv, "simulation.log_csv", "simulation_results/cesfam_log.csv");
    const std::string csv_sep = get_string(kv, "simulation.csv_sep", ";");

    // Ensure output folder exists
    try {
        std::filesystem::path out_path(out_csv);
        if (out_path.has_parent_path()) {
            std::filesystem::create_directories(out_path.parent_path());
        }
    } catch (const std::exception& e) {
        std::cerr << "[WARN] Could not create output directories: " << e.what() << "\n";
    }

    // ---- Build model & run --------------------------------------------------
    auto model = std::make_shared<CESFAM>("CESFAM", cfg);
    auto root = cadmium::RootCoordinator(model);

    cesfam_compat::attach_csv_logger(root, out_csv, csv_sep);

    root.start();
    if (until <= 0.0) {
        root.simulate(std::numeric_limits<double>::infinity());
    } else {
        root.simulate(until);
    }
    root.stop();

    std::cout << "Simulation finished. Log: " << out_csv << "\n";
    return 0;
}
