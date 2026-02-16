//   _____ __  __ ___      _                    _
//  |  ___|  \/  |_ _|    / \   __ _  ___ _ __ | |_
//  | |_  | |\/| || |    / _ \ / _` |/ _ \ '_ \| __|
//  |  _| | |  | || |   / ___ \ (_| |  __/ | | | |_
//  |_|   |_|  |_|___| /_/   \_\__, |\___|_| |_|\__|
//                             |___/

#include "../FmuInstance.hpp"
#include <mads.hpp>
#include <agent.hpp>
#include <rang.hpp>
#include <cxxopts.hpp>
#include <filesystem>
#include <chrono>

using namespace std;
using namespace chrono_literals;
using namespace cxxopts;
using json = nlohmann::json;
using namespace Mads;

int main(int argc, char * const *argv) {
  // Mads-related
  string settings_uri = SETTINGS_URI;
  bool crypto = false;
  filesystem::path key_dir(Mads::exec_dir() + "/../etc");
  string client_key_name = "client";
  string server_key_name = "broker";
  Mads::auth_verbose auth_verbose = auth_verbose::off;
  string agent_name = argv[0], agent_id;
  chrono::milliseconds period{0};
  // FMI-related
  filesystem::path fmu_path;
  FmuWrapper fmu;
  string plant_name = "plant";
  double relative_tol = 1e-5;
  double absolute_tol = 1e-7;
  double hmin = 1e-12;

  // Parse command line options ================================================
  Options options(argv[0]);
  // if needed, add here further CLI options
  // clang-format off
  options.add_options()
    ("p,period", "Sampling period (default 100 ms)", value<size_t>())
    ("f,fmu", "FMU file to load", value<filesystem::path>())
    ("n,name", "Agent name (default to 'fmu_agent')", value<string>())
    ("i,agent-id", "Agent ID to be added to JSON frames", value<string>());
  SETUP_OPTIONS(options, Agent);
  // clang-format on
  if (options_parsed.count("name") != 0) {
    agent_name = options_parsed["name"].as<string>();
  } 
  
  Agent agent(agent_name, settings_uri);
  if (options_parsed.count("agent-id")) {
    agent.set_agent_id(options_parsed["agent-id"].as<string>());
  }
  if (crypto) {
    agent.set_key_dir(key_dir);
    agent.client_key_name = client_key_name;
    agent.server_key_name = server_key_name;
    agent.auth_verbose = auth_verbose;
  }
  try {
    agent.init(crypto);
  } catch (const std::exception &e) {
    std::cout << fg::red << "Error initializing agent: " << e.what()
              << fg::reset << endl;
    exit(EXIT_FAILURE);
  }

  // Settings
  json settings = agent.get_settings();
  fmu_path = settings.value("fmu_path", "");
  if (options_parsed.count("fmu") != 0) {
    fmu_path = options_parsed["fmu"].as<filesystem::path>();
    if (fmu_path.extension() != ".fmu") {
      cerr << fg::red << "FMU file must have .fmu extension" << fg::reset 
           << endl;
      exit(EXIT_FAILURE);
    }
  }
  if (!filesystem::exists(fmu_path)) {
    cerr << fg::red << "No FMU file at path " << fmu_path.string() 
         << fg::reset << endl;
    exit(EXIT_FAILURE);
  }
  period = chrono::milliseconds(settings.value("period", 100));
  if (options_parsed.count("period") != 0) {
    period = chrono::milliseconds(options_parsed["period"].as<size_t>());
  }
  plant_name = settings.value("plant_name", plant_name);
  relative_tol = settings.value("relative_tol", relative_tol);
  absolute_tol = settings.value("absolute_tol", absolute_tol);
  hmin = settings.value("hmin", hmin);

  // Connection
  agent.enable_threaded_remote_control();
  agent.connect();
  agent.register_event(event_type::startup);
  agent.info();
  cout << "  period:           " << style::bold << period << style::reset 
       << endl
       << "  FMU file path:    " << style::bold << fmu_path << style::reset 
       << endl
       << "  relative tol:     " << style::bold << relative_tol << style::reset 
       << endl
       << "  absolutre tol:    " << style::bold << absolute_tol << style::reset 
       << endl
       << "  hmin:             " << style::bold << hmin << style::reset 
       << endl;

  FmuWrapper plant(fmu_path, "plant");
  plant._solver_params._rel_tol = 1e-5;
  plant._solver_params._abs_tol = 1e-7;
  plant._solver_params._hmin = 1e-12;

  auto last_timestep = chrono::steady_clock::now();
  chrono::steady_clock::time_point now;
  double dt = 0, t = 0;
  json status;
  agent.loop([&]() -> chrono::milliseconds {
    now = chrono::steady_clock::now();
    dt = chrono::duration_cast<chrono::microseconds>(now - last_timestep).count() / 1e6;
    last_timestep = now;
    plant.do_step(dt);
    plant.get_status(status);
    agent.publish(status);
    t += dt;
    cout << "\r\x1b[0KSent status after " << t << " s";
    cout.flush();
    return 0ms;
  }, period);
  cout << endl << fg::green << "FMU agent stopped" 
       << fg::reset << endl;
  agent.register_event(event_type::shutdown);
  agent.disconnect();
  if (agent.restart()) {
    auto cmd = string(MADS_PREFIX) + argv[0];
    cout << "Restarting " << cmd << "..." << endl;
    execvp(cmd.c_str(), argv);
  }
  return 0;
}
