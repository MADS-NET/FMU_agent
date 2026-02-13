#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <stdexcept>

extern "C" {
#include <fmi4c.h>
}

class FmuWrapper {
public:
    /// Constructor loads the FMU and sets up the instance
    /// @param fmu_path Path to the .fmu file
    /// @param instance_name Name for the FMU instance
    /// @param start_time Initial simulation time (default: 0.0)
    /// @throws std::runtime_error if FMU loading or instantiation fails
    FmuWrapper(const std::string& fmu_path,
               const std::string& instance_name,
               double start_time = 0.0);

    /// Destructor cleans up FMU resources
    ~FmuWrapper();

    // Prevent copying
    FmuWrapper(const FmuWrapper&) = delete;
    FmuWrapper& operator=(const FmuWrapper&) = delete;

    // Allow moving
    FmuWrapper(FmuWrapper&& other) noexcept;
    FmuWrapper& operator=(FmuWrapper&& other) noexcept;

    /// Execute one simulation step
    /// @param dt Step size (delta time)
    /// @throws std::runtime_error if step execution fails
    void step(double dt);

    void doStep(double dt);

    /// Set a real-valued variable by name
    /// @param name Variable name
    /// @param value Value to set
    /// @throws std::runtime_error if variable not found or set fails
    void setReal(const std::string& name, double value);

    /// Get a real-valued variable by name
    /// @param name Variable name
    /// @return Variable value
    /// @throws std::runtime_error if variable not found or get fails
    double getReal(const std::string& name) const;

    /// Get current simulation time
    /// @return Current time
    double getTime() const { return _current_time; }

    void list_variables(std::ostream &s = std::cout);

    struct SolverParams {
        double relTol = 1e-6;
        double absTol = 1e-8;
        double hmin = 1e-12;
    };

    SolverParams solver_params;

private:
    fmuHandle* _fmu = nullptr;
    fmi3InstanceHandle* _instance = nullptr;
    std::string _fmu_path, _instance_name;
    double _start_time, _current_time;
    std::unordered_map<std::string, fmi3ValueReference> _var_cache;

    // Static enum-to-string translation maps
    static const std::unordered_map<int, std::string> initialKindMap;
    static const std::unordered_map<int, std::string> causalityMap;
    static const std::unordered_map<int, std::string> dataTypeMap;

    /// Resolve variable name to value reference
    fmi3ValueReference resolveVarRef(const std::string& name) const;
    
    /// Check FMI status and throw if error
    void checkStatus(fmi3Status status, const std::string& context) const;
    
    /// Helper methods to translate enums to strings
    static std::string getInitialKindString(int kind);
    static std::string getCausalityString(int causality);
    static std::string getDataTypeString(int dataType);
    
    /// Handle discrete state updates and events
    void handleEvents();
};