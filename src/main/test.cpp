#include "../FmuInstance.hpp"
#include <iostream>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// #define JSON_OUT

using namespace std;

int main(int argc, const char **argv) {
  FmuWrapper plant(argv[1], "plant");
  // std::cerr << "variables:" << std::endl;
  // plant.list_variables(std::cerr);

  double dt = 1e-3;
  double t = 0;
  // plant.set_real("dx", 1);
  std::cout << "t" << " ";
  // for (auto const &n : plant.get_indep_names()) {
  //   cout << n << " ";
  // }
  for (auto const &n : plant.get_param_names()) {
    cout << n << " ";
  }
  for (auto const &n : plant.get_input_names()) {
    cout << n << " ";
  }
  for (auto const &n : plant.get_output_names()) {
    cout << n << " ";
  }
  cout << std::endl;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(1e-3, 1e-2);

  plant._solver_params._rel_tol = 1e-5;
  plant._solver_params._abs_tol = 1e-7;
  plant._solver_params._hmin = 1e-12;

  map<string, double> inputs;
  map<string, double> outs;
  map<string, double> params;
  map<string, double> indep;
  json status;
  plant.set_real("setpoint", 0.5);
  while (t < 10) {
    dt = dis(gen);
    plant.do_step(dt);
    t += dt;
    #ifdef JSON_OUT
    plant.get_status(status);
    cout << status.dump() << endl;
    #else
    plant.get_indep(indep);
    plant.get_inputs(inputs);
    plant.get_params(params);
    plant.get_outputs(outs);
    // if (t > 5)
    //   plant.set_real("b", 50);
    cout << t << " ";
    // for (auto const &[n, v] : indep) {
    //   cout << v << " ";
    // }
    for (auto const &[n, v] : params) {
      cout << v << " ";
    }
    for (auto const &[n, v] : inputs) {
      cout << v << " ";
    }
    for (auto const &[n, v] : outs) {
      cout << v << " ";
    }
    std::cout << std::endl;
    #endif
    // feed y back into MADS
  }
  return 0;
}
