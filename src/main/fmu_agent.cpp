//   _____ __  __ ___      _                    _
//  |  ___|  \/  |_ _|    / \   __ _  ___ _ __ | |_
//  | |_  | |\/| || |    / _ \ / _` |/ _ \ '_ \| __|
//  |  _| | |  | || |   / ___ \ (_| |  __/ | | | |_
//  |_|   |_|  |_|___| /_/   \_\__, |\___|_| |_|\__|
//                             |___/

#include "../FmuInstance.hpp"
#include "../goback.hpp"
#include <agent.hpp>
#include <chrono>
#include <cmath>
#include <cxxopts.hpp>
#include <filesystem>
#include <mads.hpp>
#include <rang.hpp>
#include <array>
#ifndef __linux__
#include <format>
#endif 

using namespace std;
using namespace chrono_literals;
using namespace cxxopts;
using json = nlohmann::json;
using namespace Mads;

bool inspect_fmu(filesystem::path const &path) {
  if (!filesystem::exists(path)) {
    cout << fg::red << "No file at " << path << fg::reset << endl;
    return false;
  }
  FmuWrapper fmu(path.string(), "plant");
  fmu.initialize();
  const FmuWrapper::SolverParams defaults;
  auto model_name = fmu.model_name();
  
  cout << style::bold 
       << "Inspecting model at:    " << fg::green << path.string() << fg::reset
       << "\nModel name (from .fmu): " << fg::green << model_name << fg::reset
       << "\nAgent name (default):   " << fg::green << "fmu_" << model_name
       << fg::reset << style::reset << endl;
  fmu.list_variables(cout);
  cout << endl
       << style::bold << "INI settings section (defaults):" << endl
       << style::reset << fg::green
       << "[fmu_" << model_name << "]"
       << "\nperiod = 100"
       << "\npub_topic = \"fmu_" <<  model_name << "\""
       << "\nsub_topic = [\"" <<  model_name << "_control\"]"
       << "\nrelative_tol = " << defaults._rel_tol
       << "\nabsolute_tol = " << defaults._abs_tol
       << "\nhmin = " << defaults._hmin
       << "\n[fmu_" << model_name << ".parameters]"
       << "\n# default values from FMU";
  for (const auto &param : fmu.get_param_names()) {
    cout << "\n" << param << " = " << fmu.value(param).dump();
  }
  cout << fg::reset << endl;

  cout << style::bold << "\nDependencies:" << style::reset << endl;
  auto deps = fmu.get_binary_dependencies();
  for (auto &s : deps) {
    cout << "  " << s << endl;
  }
  return true;
}

int main(int argc, char *const *argv) {
  // Mads-related
  string settings_uri = SETTINGS_URI;
  bool crypto = false;
  bool trigger_mode = false;
  filesystem::path key_dir(Mads::exec_dir() + "/../etc");
  string client_key_name = "client";
  string server_key_name = "broker";
  Mads::auth_verbose auth_verbose = auth_verbose::off;
  string agent_name = argv[0], agent_id;
  chrono::milliseconds period{100};
  // FMI-related
  filesystem::path fmu_path;

  // Parse command line options ================================================
  Options options(argv[0]);
  // if needed, add here further CLI options
  // clang-format off
  options.add_options()
    ("f, fmu", "FMU file to load", value<filesystem::path>())
    ("p,period", "Sampling period (default 100 ms)", value<size_t>())
    ("n,name", "Agent name (default to fmu_<model name>)", value<string>())
    ("t,trigger", "Enable trigger mode")
    ("i,agent-id", "Agent ID to be added to JSON frames", value<string>())
    ("inspect", "Inspect the FMU and exit");
  // clang-format on
  options.parse_positional({"fmu"});
  options.positional_help("<module.fmu>");
  SETUP_OPTIONS(options, Agent);

  if (options_parsed.count("fmu") != 0) {
    fmu_path = options_parsed["fmu"].as<filesystem::path>();
    if (fmu_path.extension() != ".fmu") {
      cerr << fg::red << "FMU file must have .fmu extension" << fg::reset
           << endl;
      exit(EXIT_FAILURE);
    }
    if (!filesystem::exists(fmu_path)) {
      cerr << fg::red << "No FMU file at path " << fmu_path.string()
           << fg::reset << endl;
      exit(EXIT_FAILURE);
    }
  } else {
    cerr << fg::red << "Missing mandatory FMU path" << fg::reset << endl;
    exit(EXIT_FAILURE);
  }
  if (options_parsed.count("inspect") != 0) {
    if (inspect_fmu(fmu_path))
      exit(EXIT_SUCCESS);
    else
      exit(EXIT_FAILURE);
  }
  
  // Load FMU
  FmuWrapper plant(fmu_path.string(), "plant");
  agent_name = string("fmu_") + plant.model_name();
  if (options_parsed.count("name") != 0) {
    agent_name = options_parsed["name"].as<string>();
  }

  // Create Agent
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
  period = chrono::milliseconds(settings.value("period", 100));
  if (options_parsed.count("period") != 0) {
    period = chrono::milliseconds(options_parsed["period"].as<size_t>());
  }
  auto &solver = plant._solver_params;
  solver._rel_tol = settings.value("relative_tol", solver._rel_tol);
  solver._abs_tol = settings.value("absolute_tol", solver._abs_tol);
  solver._hmin = settings.value("hmin", solver._hmin);

  trigger_mode = settings.value("trigger_mode", trigger_mode);
  if (options_parsed.count("trigger") != 0) {
    trigger_mode = true;
  }

  // Parameters must be set before initialization, where some of them are
  // read. Also used after a reset, which restores the FMU defaults.
  auto init_plant = [&]() {
    if (settings.contains("parameters")) {
      for (auto const &[param, value] : settings["parameters"].items()) {
        plant.set(param, value);
      }
    }
    plant.initialize();
  };
  try {
    init_plant();
  } catch (const std::exception &e) {
    cerr << fg::red << "Error initializing FMU: " << e.what() << fg::reset
         << endl;
    exit(EXIT_FAILURE);
  }

  // Fixed-step co-simulation FMUs: times are counted in whole microseconds
  const long long period_us =
      chrono::duration_cast<chrono::microseconds>(period).count();
  const long long step_us =
      plant.get_fixed_step() ? llround(plant.get_step_size() * 1e6) : 0;
  if (plant.get_fixed_step()) {
    if (step_us <= 0) {
      cerr << fg::red << "The model's fixed step is below 1 us" << fg::reset
           << endl;
      exit(EXIT_FAILURE);
    }
    if (!trigger_mode && period_us % step_us != 0) {
      cerr << fg::red << "The agent period (" << period.count()
           << " ms) must be a multiple of the model's fixed step ("
           << plant.get_step_size() << " s)" << fg::reset << endl;
      exit(EXIT_FAILURE);
    }
  }

  // Connection
  agent.enable_remote_control();
  agent.connect();
  agent.register_event(event_type::startup);
  agent.info();
  if (trigger_mode) {
    period = 0ms; // not used in trigger mode, but set to 0 for clarity in output
    cout << "  period:           " << style::bold << fg::yellow 
         << "Trigger mode enabled" << fg::reset
         << style::reset << endl;
  } else {
    cout << "  period:           " << style::bold << period.count() << " ms" 
         << style::reset << endl;
  }
  cout << "  FMU file path:    " << style::bold << fmu_path << style::reset
       << endl
       << "  FMU type:          " << style::bold << ((plant.get_type() == 1) ? "Model Exchange" : "Co-Simulation") << style::reset << endl
       << "  relative tol:     " << style::bold << solver._rel_tol
       << style::reset << endl
       << "  absolute tol:     " << style::bold << solver._abs_tol
       << style::reset << endl
       << "  hmin:             " << style::bold << solver._hmin << style::reset
       << endl;

  if (settings.contains("parameters")) {
    cout << style::bold << "FMU parameters from settings ("
         << settings["parameters"].size() << "):" << style::reset << endl;
    for (auto const &[param, value] : settings["parameters"].items()) {
      cout << "  " << param << ": " << style::bold << value << style::reset
           << endl;
    }
  }
  // Room for the status lines redrawn with goback()
  cout << string(trigger_mode ? 4 : 3, '\n');

  auto last_timestep = chrono::steady_clock::now();
  chrono::steady_clock::time_point now;
  double dt = 0;
  long long t_buffer_us = 0;
  bool first_trigger = true;
  json status;
  array<string, 3> console_out;
  // The wrapper owns the simulation clock
  auto sim_time = [&]() { return to_string(plant.get_time()) + " s"; };

  // Process a received JSON message: update plant inputs and console_out.
  // early_exit=true (free-running): a reset skips fmu_input for that message.
  // early_exit=false (trigger): fmu_input is applied after the reset.
  auto process_input = [&](json const &in, bool early_exit) {
    if (!in.is_object()) {
      console_out[0] = sim_time() + ", Message is not a JSON object";
      return;
    }
    const bool reset = in.value("fmu_reset", false);
    if (reset) {
      plant.reset();
      init_plant();
      console_out[0] = sim_time() + ", Reset received";
      if (early_exit) return;
    }
    if (!in.contains("fmu_input") || !in["fmu_input"].is_object()) {
      if (!reset) console_out[0] = sim_time() + ", Missing fmu_input";
      return;
    }
    console_out[1] = sim_time() + ", " + in["fmu_input"].dump();
    for (auto const &[k, v] : in["fmu_input"].items()) {
      try {
        plant.set(k, v);
      } catch (const std::exception &e) {
        // One bad variable must not block the others, nor stop the agent
        console_out[1] += string(" (") + e.what() + ")";
      }
    }
  };

  // A malformed message is reported and dropped
  auto handle_message = [&](string const &payload, bool early_exit) {
    try {
      process_input(json::parse(payload), early_exit);
    } catch (const std::exception &e) {
      console_out[0] = sim_time() + ", Message ignored: " + e.what();
    }
  };

  if (trigger_mode) {
    agent.loop([&]() -> chrono::milliseconds {
      if (agent.receive(false) == message_type::json &&
          agent.last_topic() != "control") {
        const string payload = get<1>(agent.last_message());
        now = chrono::steady_clock::now();
        if (first_trigger) { // the wait for the first message is not simulated
          last_timestep = now;
          first_trigger = false;
        }
        long long dt_us =
            chrono::duration_cast<chrono::microseconds>(now - last_timestep)
                .count();
        last_timestep = now;

        bool step = true;
        if (plant.get_fixed_step()) {
          t_buffer_us += dt_us;
          dt_us = (t_buffer_us / step_us) * step_us; // whole model steps only
          t_buffer_us -= dt_us;                      // keep the remainder
          step = dt_us > 0;
        }
        if (step) {
          dt = dt_us / 1e6;
          plant.do_step(dt);
          plant.get_status(status);
          auto tet = chrono::steady_clock::now() - now;
          status["TET"] =
              chrono::duration_cast<chrono::microseconds>(tet).count();
          agent.publish(status);
          console_out[2] = sim_time();
        } else {
          console_out[2] =
              "skipped, " + to_string(t_buffer_us / 1e6) + " s pending";
        }
        // Inputs are applied after the step, also when the step is skipped
        handle_message(payload, false);
      }

      cout << goback(4) << fg::yellow << "Last message: " << console_out[0]
        << fg::reset << endl
        << "Received: " << console_out[1] << endl
        << "Status update after: " << console_out[2] << endl
        << "Simulation dt: " << dt << "s" << endl;
      
      return 0ms;
    });
  } else {
    agent.loop([&]() -> chrono::milliseconds {
      // timing
      now = chrono::steady_clock::now();
      if (plant.get_fixed_step()) {
        dt = period_us / 1e6;
      } else {
        dt = chrono::duration_cast<chrono::microseconds>(now - last_timestep)
                 .count() / 1e6;
        last_timestep = now;
      }
      // input: drain the queue, or with more than one message per period the
      // plant would run on stale inputs
      for (auto type = agent.receive(true); type != message_type::none;
           type = agent.receive(true)) {
        if (type == message_type::json && agent.last_topic() != "control") {
          handle_message(get<1>(agent.last_message()), true);
        }
      }
      // Integrate
      plant.do_step(dt);
      plant.get_status(status);
      console_out[2] = sim_time();
      auto tet = chrono::steady_clock::now() - now;
      status["TET"] =
          chrono::duration_cast<chrono::microseconds>(tet).count();
      // output
      agent.publish(status);
      cout << goback(3) << fg::yellow << "Last message: " << console_out[0]
            << fg::reset << endl
            << "Received: " << console_out[1] << endl
            << "Status update after: " << console_out[2] << endl;
      return 0ms;
    }, period);
  }
  cout << endl << fg::green << "FMU agent stopped" << fg::reset << endl;
  agent.register_event(event_type::shutdown);
  agent.disconnect();
  if (agent.restart()) {
    auto cmd = string(MADS_PREFIX) + argv[0];
    cout << "Restarting " << cmd << "..." << endl;
    execvp(cmd.c_str(), argv);
  }
  return 0;
}
