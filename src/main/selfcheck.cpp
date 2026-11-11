// Smoke test of FmuWrapper on the bundled example FMUs.
// Usage: fmu-selfcheck <directory holding linear_axis.fmu, machine_tool.fmu
//                       and DoubleMassSpringDamper.fmu>
#include "../FmuInstance.hpp"
#include <cmath>
#include <iostream>

static int failures = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      ++failures;                                                              \
      std::cerr << "FAIL line " << __LINE__ << ": " #cond << std::endl;        \
    }                                                                          \
  } while (0)

static bool near(double value, double expected, double rel) {
  return std::abs(value - expected) <= rel * std::abs(expected);
}

template <typename F> static bool throws(F f) {
  try {
    f();
  } catch (const std::exception &) {
    return true;
  }
  return false;
}

static void run(FmuWrapper &plant, int steps, double dt) {
  for (int i = 0; i < steps; ++i) {
    plant.do_step(dt);
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <fmu directory>" << std::endl;
    return 2;
  }
  const std::string dir = argv[1];

  { // linear_axis: step response, model time, reset
    FmuWrapper plant(dir + "/linear_axis.fmu", "plant");
    CHECK(throws([&] { plant.do_step(0.1); })); // not initialized yet
    plant.initialize();
    CHECK(plant.get_real("X") == 0.0);
    plant.set("setpoint", 100.0);
    run(plant, 10, 0.1);
    const double x1 = plant.get_real("X");
    // Reference 63.2640717 computed with relative_tol 1e-9
    CHECK(near(x1, 63.2640717, 1e-5));
    CHECK(near(plant.get_time(), 1.0, 1e-12));
    CHECK(plant.get_real("time") == plant.get_time()); // model time follows

    plant.reset();
    plant.initialize();
    CHECK(plant.get_time() == 0.0);
    CHECK(plant.get_real("time") == 0.0);
    CHECK(plant.get_real("X") == 0.0);
    CHECK(plant.get_real("setpoint") == 0.0); // inputs back to start values
    plant.set("setpoint", 100.0);
    run(plant, 10, 0.1);
    CHECK(plant.get_real("X") == x1); // a reset run repeats exactly

    // Uneven steps, as with a wall-clock period, give the same trajectory
    plant.reset();
    plant.initialize();
    plant.set("setpoint", 100.0);
    for (double dt : {0.013, 0.2, 0.0007, 0.05, 0.1363, 0.3, 0.3}) {
      plant.do_step(dt);
    }
    CHECK(near(plant.get_time(), 1.0, 1e-12));
    CHECK(near(plant.get_real("X"), 63.2640717, 1e-5));

    plant._solver_params._hmin = 1.0; // cannot be met: must fail loudly
    CHECK(throws([&] { plant.do_step(0.1); }));
  }

  { // linear_axis: parameters before initialization, Boolean input
    FmuWrapper plant(dir + "/linear_axis.fmu", "plant");
    plant.set("X0", 0.05);
    plant.initialize();
    CHECK(near(plant.get_real("X"), 50.0, 1e-12));
    plant.set("setpoint", 100.0);
    run(plant, 10, 0.1);
    CHECK(plant.get_real("X") > 60.0);
    plant.set("reset", true); // rising edge re-reads X0
    plant.do_step(0.0);
    CHECK(near(plant.get_real("X"), 50.0, 1e-12));
    CHECK(throws([&] { plant.set("reset", 1.0); }));    // Boolean needs a bool
    CHECK(throws([&] { plant.set("setpoint", true); })); // needs a number
    CHECK(throws([&] { plant.set("no_such_variable", 1.0); }));

    plant.reset();
    plant.set("X0", 0.05);
    plant.initialize();
    CHECK(near(plant.get_real("X"), 50.0, 1e-12));
  }

  { // machine_tool: array variables
    FmuWrapper plant(dir + "/machine_tool.fmu", "plant");
    plant.initialize();
    CHECK(throws([&] { plant.set("setpoint", 5.0); }));   // scalar into array
    CHECK(throws([&] { plant.set("setpoint", json::array({1.0, 2.0})); }));
    CHECK(throws([&] { plant.get_real("position"); }));   // array as scalar
    plant.set("setpoint", json::array({100.0, 50.0, 20.0}));
    run(plant, 20, 0.1);
    json status;
    plant.get_status(status);
    CHECK(status["output"]["position"].is_array());
    CHECK(status["output"]["position"].size() == 3);
    CHECK(status["input"]["setpoint"] == json::array({100.0, 50.0, 20.0}));
    CHECK(status["input"]["reset"].is_boolean());
    CHECK(near(status["independent"]["time"].get<double>(), 2.0, 1e-12));
    // Reference 0.219303848 computed with relative_tol 1e-9
    const double x = status["output"]["position"][0].get<double>();
    CHECK(near(x, 0.219303848, 5e-5));
  }

  { // DoubleMassSpringDamper: no inputs, no events
    FmuWrapper plant(dir + "/DoubleMassSpringDamper.fmu", "plant");
    plant.initialize();
    run(plant, 20, 0.1);
    // Reference -1.07725262 computed with relative_tol 1e-9
    CHECK(near(plant.get_real("s1"), -1.07725262, 1e-5));
  }

  std::cout << (failures ? "FAILED" : "OK") << ": " << failures << " failure(s)"
            << std::endl;
  return failures ? 1 : 0;
}
