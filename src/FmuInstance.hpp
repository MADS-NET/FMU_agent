#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

extern "C" {
#include <fmi4c.h>
}

using json = nlohmann::json;

class FmuWrapper {
public:
    FmuWrapper() {}

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

    std::string model_name();

    void reset();

    /// Execute one simulation step
    /// @param dt Step size (delta time)
    /// @throws std::runtime_error if step execution fails
    void do_step(double dt);

    /// Set a real-valued variable by name
    /// @param name Variable name
    /// @param value Value to set
    /// @throws std::runtime_error if variable not found or set fails
    void set_real(const std::string& name, double value);

    /// Get a real-valued variable by name
    /// @param name Variable name
    /// @return Variable value
    /// @throws std::runtime_error if variable not found or get fails
    double get_real(const std::string& name) const;

    /// Get current simulation time
    /// @return Current time
    double get_time() const { return _current_time; }

    size_t get_outputs(std::map<std::string, double> &outs) const;
    size_t get_inputs(std::map<std::string, double> &ins) const;
    size_t get_params(std::map<std::string, double> &params) const;
    size_t get_indep(std::map<std::string, double> &indep) const;
    
    json value(std::string const &n);
    void get_status(json &status);

    void list_variables(std::ostream &s = std::cout);

    std::vector<std::string> get_param_names() const;
    std::vector<std::string> get_input_names() const;
    std::vector<std::string> get_output_names() const;
    std::vector<std::string> get_indep_names() const;

    struct SolverParams {
        double _rel_tol = 1e-6;
        double _abs_tol = 1e-8;
        double _hmin = 1e-12;
    };

    SolverParams _solver_params;

private:
    fmuHandle* _fmu = nullptr;
    fmi3InstanceHandle* _instance = nullptr;
    std::string _fmu_path = "", _instance_name = "";
    bool _initialized = false;
    std::vector<std::string> _out_vars;
    std::vector<std::string> _in_vars;
    std::vector<std::string> _param_vars;
    std::vector<std::string> _indep_vars;

    double _start_time, _current_time;
    std::unordered_map<std::string, fmi3ValueReference> _var_cache;

    // Static enum-to-string translation maps
    static const std::unordered_map<int, std::string> _initial_kind_map;
    static const std::unordered_map<fmi3Causality, std::string> _causality_map;
    static const std::unordered_map<fmi3DataType, std::string> _data_type_map;

    /// Resolve variable name to value reference
    fmi3ValueReference resolve_var_ref(const std::string& name) const;
    
    /// Check FMI status and throw if error
    void check_status(fmi3Status status, const std::string& context) const;
    
    /// Helper methods to translate enums to strings
    static std::string get_initial_kind_string(fmi3Initial kind);
    static std::string get_causality_string(fmi3Causality causality);
    static std::string get_data_type_string(fmi3DataType dataType);
    
    /// Handle discrete state updates and events
    void handle_events();
};
