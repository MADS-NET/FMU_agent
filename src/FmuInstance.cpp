#include "FmuInstance.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

// Static map initializations for enum-to-string translation
const std::unordered_map<int, std::string> FmuWrapper::initialKindMap = {
    {0, "exact"},
    {1, "approx"},
    {2, "calculated"}
};

const std::unordered_map<int, std::string> FmuWrapper::causalityMap = {
    {0, "structuralParameter"},
    {1, "output"},
    {2, "input"},
    {3, "independent"},
    {4, "calculatedParameter"},
    {5, "parameter"},
    {6, "unknown"}
};

const std::unordered_map<int, std::string> FmuWrapper::dataTypeMap = {
    {0, "Float64"},
    {1, "Float32"},
    {2, "Int8"},
    {3, "UInt8"},
    {4, "Int16"},
    {5, "UInt16"},
    {6, "Int32"},
    {7, "UInt32"},
    {8, "Int64"},
    {9, "UInt64"},
    {10, "Boolean"},
    {11, "String"},
    {12, "Binary"},
    {13, "Enumeration"},
    {14, "Clock"}
};

// Helper methods to translate enums to strings
std::string FmuWrapper::getInitialKindString(int kind) {
  auto it = initialKindMap.find(kind);
  return (it != initialKindMap.end()) ? it->second : "unknown(" + std::to_string(kind) + ")";
}

std::string FmuWrapper::getCausalityString(int causality) {
  auto it = causalityMap.find(causality);
  return (it != causalityMap.end()) ? it->second : "unknown(" + std::to_string(causality) + ")";
}

std::string FmuWrapper::getDataTypeString(int dataType) {
  auto it = dataTypeMap.find(dataType);
  return (it != dataTypeMap.end()) ? it->second : "unknown(" + std::to_string(dataType) + ")";
}

FmuWrapper::FmuWrapper(const std::string &fmu_path,
                       const std::string &instance_name, double start_time)
    : _fmu_path(fmu_path), _instance_name(instance_name),
      _start_time(start_time), _current_time(start_time) {
  // Load the FMU file
  _fmu = fmi4c_loadFmu(fmu_path.c_str(), instance_name.c_str());

  if (!_fmu) {
    throw std::runtime_error("Failed to load FMU: " + fmu_path);
  }

  auto v = fmi4c_getFmiVersion(_fmu);
  if (v != 3) {
    throw std::runtime_error(std::string("Unsupported FMU version: ") + std::to_string(v));
  }

  // Instantiate FMI3 CoSimulation component
  _instance =
      fmi3_instantiateModelExchange(_fmu, false, false, nullptr, nullptr);

  if (!_instance) {
    fmi4c_freeFmu(_fmu);
    _fmu = nullptr;
    throw std::runtime_error("Failed to instantiate FMU: " + instance_name);
  }

  // Enter initialization mode
  fmi3Status status = fmi3_enterInitializationMode(_instance,
                                                   false, // toleranceDefined
                                                   0.0,   // tolerance
                                                   start_time, // startTime
                                                   false, // stopTimeDefined
                                                   0.0    // stopTime
  );
  checkStatus(status, "enterInitializationMode");

  // Exit initialization mode to prepare for stepping
  status = fmi3_exitInitializationMode(_instance);
  checkStatus(status, "exitInitializationMode");
}

FmuWrapper::~FmuWrapper() {
  if (_instance) {
    // Terminate simulation
    fmi3_terminate(_instance);
    fmi3_freeInstance(_instance);
    _instance = nullptr;
  }
  if (_fmu) {
    fmi4c_freeFmu(_fmu);
    _fmu = nullptr;
  }
}

FmuWrapper::FmuWrapper(FmuWrapper &&other) noexcept
    : _fmu(other._fmu), _instance(other._instance),
      _fmu_path(std::move(other._fmu_path)),
      _instance_name(std::move(other._instance_name)),
      _start_time(other._start_time), _current_time(other._current_time),
      _var_cache(std::move(other._var_cache)) {
  other._fmu = nullptr;
  other._instance = nullptr;
}

FmuWrapper &FmuWrapper::operator=(FmuWrapper &&other) noexcept {
  if (this != &other) {
    // Clean up current resources
    if (_instance) {
      fmi3_terminate(_instance);
      fmi3_freeInstance(_instance);
    }
    if (_fmu) {
      fmi4c_freeFmu(_fmu);
    }

    // Move from other
    _fmu = other._fmu;
    _instance = other._instance;
    _fmu_path = std::move(other._fmu_path);
    _instance_name = std::move(other._instance_name);
    _start_time = other._start_time;
    _current_time = other._current_time;
    _var_cache = std::move(other._var_cache);

    other._fmu = nullptr;
    other._instance = nullptr;
  }
  return *this;
}

void FmuWrapper::step(double dt) {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }

  // Get the number of continuous states
  size_t numStates = 0;
  fmi3Status status = fmi3_getNumberOfContinuousStates(_instance, &numStates);
  checkStatus(status, "getNumberOfContinuousStates");
  
  if (numStates == 0) {
    // No states to integrate, just advance time and handle events
    _current_time += dt;
    handleEvents();
    return;
  }

  // Adaptive RK45 integration parameters
  const double &relTol = solver_params.relTol;
  const double &absTol = solver_params.absTol;
  const double &hmin = solver_params.hmin;
  
  double t = _current_time;
  double tend = _current_time + dt;
  double h = dt; // Initial step size
  
  std::vector<fmi3Float64> y(numStates);
  std::vector<fmi3Float64> ytemp(numStates);
  std::vector<fmi3Float64> k1(numStates), k2(numStates), k3(numStates);
  std::vector<fmi3Float64> k4(numStates), k5(numStates), k6(numStates);
  std::vector<fmi3Float64> yerr(numStates);
  
  // Get initial state
  status = fmi3_getContinuousStates(_instance, y.data(), numStates);
  checkStatus(status, "getContinuousStates");
  
  // Adaptive stepping loop
  while (t < tend && h > hmin) {
    // Limit step to not overshoot tend
    if (t + h > tend) {
      h = tend - t;
    }
    
    // RK45 with Dormand-Prince coefficients
    // Stage 1: k1 = f(t, y)
    status = fmi3_getContinuousStateDerivatives(_instance, k1.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 1");
    
    // Stage 2: k2 = f(t + (1/5)*h, y + (1/5)*h*k1)
    for (size_t i = 0; i < numStates; ++i) {
      ytemp[i] = y[i] + (h / 5.0) * k1[i];
    }
    status = fmi3_setContinuousStates(_instance, ytemp.data(), numStates);
    checkStatus(status, "setContinuousStates at stage 2");
    status = fmi3_getContinuousStateDerivatives(_instance, k2.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 2");
    
    // Stage 3: k3 = f(t + (3/10)*h, y + (3/40)*h*k1 + (9/40)*h*k2)
    for (size_t i = 0; i < numStates; ++i) {
      ytemp[i] = y[i] + (3.0/40.0) * h * k1[i] + (9.0/40.0) * h * k2[i];
    }
    status = fmi3_setContinuousStates(_instance, ytemp.data(), numStates);
    checkStatus(status, "setContinuousStates at stage 3");
    status = fmi3_getContinuousStateDerivatives(_instance, k3.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 3");
    
    // Stage 4: k4 = f(t + (4/5)*h, y + (44/45)*h*k1 - (56/15)*h*k2 + (32/9)*h*k3)
    for (size_t i = 0; i < numStates; ++i) {
      ytemp[i] = y[i] + (44.0/45.0) * h * k1[i] - (56.0/15.0) * h * k2[i] + (32.0/9.0) * h * k3[i];
    }
    status = fmi3_setContinuousStates(_instance, ytemp.data(), numStates);
    checkStatus(status, "setContinuousStates at stage 4");
    status = fmi3_getContinuousStateDerivatives(_instance, k4.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 4");
    
    // Stage 5: k5 = f(t + (8/9)*h, y + ...)
    for (size_t i = 0; i < numStates; ++i) {
      ytemp[i] = y[i] + (19372.0/6561.0) * h * k1[i] - (25360.0/2187.0) * h * k2[i] 
               + (64448.0/6561.0) * h * k3[i] - (212.0/729.0) * h * k4[i];
    }
    status = fmi3_setContinuousStates(_instance, ytemp.data(), numStates);
    checkStatus(status, "setContinuousStates at stage 5");
    status = fmi3_getContinuousStateDerivatives(_instance, k5.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 5");
    
    // Stage 6: k6 = f(t + h, y + ...)
    for (size_t i = 0; i < numStates; ++i) {
      ytemp[i] = y[i] + (9017.0/3168.0) * h * k1[i] - (355.0/33.0) * h * k2[i] 
               + (46732.0/5247.0) * h * k3[i] + (49.0/176.0) * h * k4[i] - (5103.0/18656.0) * h * k5[i];
    }
    status = fmi3_setContinuousStates(_instance, ytemp.data(), numStates);
    checkStatus(status, "setContinuousStates at stage 6");
    status = fmi3_getContinuousStateDerivatives(_instance, k6.data(), numStates);
    checkStatus(status, "getContinuousStateDerivatives at stage 6");
    
    // 5th order solution: y_new = y + h * (35/384*k1 + 500/1113*k3 + 125/192*k4 - 2187/6784*k5 + 11/84*k6)
    // 4th order solution for error: y_hat = y + h * (5179/57600*k1 + 7571/16695*k3 + 393/640*k4 - 92097/339200*k5 + 187/2100*k6)
    std::vector<fmi3Float64> ynew(numStates);
    for (size_t i = 0; i < numStates; ++i) {
      ynew[i] = y[i] + h * (35.0/384.0 * k1[i] + 500.0/1113.0 * k3[i] + 125.0/192.0 * k4[i] 
                           - 2187.0/6784.0 * k5[i] + 11.0/84.0 * k6[i]);
      
      // Error estimate (difference between 5th and 4th order)
      double y4th = y[i] + h * (5179.0/57600.0 * k1[i] + 7571.0/16695.0 * k3[i] + 393.0/640.0 * k4[i] 
                               - 92097.0/339200.0 * k5[i] + 187.0/2100.0 * k6[i]);
      yerr[i] = std::abs(ynew[i] - y4th);
    }
    
    // Compute maximum relative error
    double maxError = 0.0;
    for (size_t i = 0; i < numStates; ++i) {
      double scale = absTol + relTol * std::abs(y[i]);
      double error = yerr[i] / scale;
      maxError = std::max(maxError, error);
    }
    
    // Adaptive step size control
    if (maxError <= 1.0) {
      // Step accepted: update y and t
      y = ynew;
      t += h;
      _current_time = t;
      
      // Set accepted state into FMU
      status = fmi3_setContinuousStates(_instance, y.data(), numStates);
      checkStatus(status, "setContinuousStates (accepted step)");
      
      // Increase step size for next iteration (but not too aggressively)
      h *= 0.9 * std::pow(1.0 / maxError, 0.2);
      h = std::min(h, 10.0 * (tend - t)); // Don't let h grow too much
    } else {
      // Step rejected: reduce step size
      h *= 0.9 * std::pow(1.0 / maxError, 0.25);
    }
    
    // Enforce minimum step size to prevent infinite loops
    if (h < hmin) {
      h = hmin;
    }
  }
  
  // Ensure we're exactly at tend
  _current_time = tend;
  
  // Handle events at the end of the step
  handleEvents();
}

void FmuWrapper::handleEvents() {
  // Check for events after stepping
  fmi3Boolean newDiscreteStatesNeeded = true;
  fmi3Boolean terminateSimulation = false;
  fmi3Boolean nominalsOfContinuousStatesChanged = false;
  fmi3Boolean valuesOfContinuousStatesChanged = false;
  fmi3Boolean nextEventTimeDefined = false;
  fmi3Float64 nextEventTime = 0.0;
  
  // Process events if necessary
  while (newDiscreteStatesNeeded) {
    fmi3Status status = fmi3_updateDiscreteStates(_instance,
                                                  &newDiscreteStatesNeeded,
                                                  &terminateSimulation,
                                                  &nominalsOfContinuousStatesChanged,
                                                  &valuesOfContinuousStatesChanged,
                                                  &nextEventTimeDefined,
                                                  &nextEventTime);
    checkStatus(status, "updateDiscreteStates");
    
    if (terminateSimulation) {
      throw std::runtime_error("FMU requested termination during event handling");
    }
  }
}

void FmuWrapper::setReal(const std::string &name, double value) {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolveVarRef(name);

  fmi3Status status = fmi3_setFloat64(_instance, &vr, 1, &value, 1);
  checkStatus(status, std::string("setFloat64 for variable ") + name);
}

double FmuWrapper::getReal(const std::string &name) const {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolveVarRef(name);
  fmi3Float64 value = 0.0;

  fmi3Status status = fmi3_getFloat64(_instance, &vr, 1, &value, 1);
  checkStatus(status, std::string("getFloat64 for variable ") + name);

  return value;
}

void FmuWrapper::list_variables(ostream &s) {
  auto &fmu = _instance->fmu;
  int vnum = fmi3_getNumberOfVariables(fmu);
  for (int i = 0; i < vnum; ++i) {
    auto v = fmi3_getVariableByIndex(fmu, i + 1);
    s << i << ": " << fmi3_getVariableName(v) 
      << " [" << fmi3_getVariableDescription(v) << "]"
      << ", initial: " << getInitialKindString(fmi3_getVariableInitial(v))
      << ", causality: " << getCausalityString(fmi3_getVariableCausality(v))
      << ", type: " << getDataTypeString(fmi3_getVariableDataType(v))
      << endl;
  }
}

fmi3ValueReference FmuWrapper::resolveVarRef(const std::string &name) const {
  // Check cache first
  auto it = _var_cache.find(name);
  if (it != _var_cache.end()) {
    return it->second;
  }

  // Look up variable in FMU
  fmi3VariableHandle *var = fmi3_getVariableByName(_fmu, name.c_str());
  if (!var) {
    throw std::runtime_error("Variable not found: " + name);
  }

  fmi3ValueReference vr =
      static_cast<fmi3ValueReference>(fmi3_getVariableValueReference(var));

  // Cache for future use
  const_cast<FmuWrapper *>(this)->_var_cache[name] = vr;

  return vr;
}

void FmuWrapper::checkStatus(fmi3Status status,
                             const std::string &context) const {
  if (status == fmi3OK || status == fmi3Warning) {
    return;
  }

  std::ostringstream oss;
  oss << "FMI error in " << context << ": ";

  switch (status) {
  case fmi3Discard:
    oss << "fmi3Discard";
    break;
  case fmi3Error:
    oss << "fmi3Error";
    break;
  case fmi3Fatal:
    oss << "fmi3Fatal";
    break;
  default:
    oss << "Unknown error code";
  }

  throw std::runtime_error(oss.str());
}