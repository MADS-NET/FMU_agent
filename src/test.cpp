#include "FmuInstance.hpp"
#include <iostream>
#include <random>

int main(int argc, const char **argv) {
  FmuWrapper plant(argv[1], "plant");
  std::cerr << "variables:" << std::endl;
  plant.list_variables(std::cerr);

  double dt = 1e-3;
  double t = 0;
  // plant.setReal("dx", 1);
  std::cout << "t" << " " << "x" << " " << "dx" << std::endl;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(1e-3, 1e-2);

  plant.solver_params.relTol = 1e-5;
  plant.solver_params.absTol = 1e-7;
  plant.solver_params.hmin = 1e-12;

  while (t < 10) {
    dt = dis(gen);
    plant.step(dt);
    t += dt;
    double x = plant.getReal("Out1");
    double dx = plant.getReal("Out2");
    std::cout << t << " " << x << " " << dx << std::endl;
    // feed y back into MADS
  }
  return 0;
}