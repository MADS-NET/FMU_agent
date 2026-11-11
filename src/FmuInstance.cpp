#include "FmuInstance.hpp"
#include <rang.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;
using namespace rang;
namespace fs = std::filesystem;

namespace {

std::string trim_copy(const std::string &s) {
  const auto begin = s.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(begin, end - begin + 1);
}

std::string shell_quote_posix(const std::string &s) {
  std::string out = "'";
  for (char c : s) {
    if (c == '\'') {
      out += "'\"'\"'";
    } else {
      out += c;
    }
  }
  out += "'";
  return out;
}

std::string read_text_file(const fs::path &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Unable to open file: " + path.string());
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

std::string run_command_capture(const std::string &command);

bool extract_xml_attribute(const std::string &tag, const std::string &attr,
                           std::string &value) {
  const std::string needle = attr + "=";
  const std::size_t key_pos = tag.find(needle);
  if (key_pos == std::string::npos) {
    return false;
  }
  const std::size_t quote_pos = key_pos + needle.size();
  if (quote_pos >= tag.size() ||
      (tag[quote_pos] != '"' && tag[quote_pos] != '\'')) {
    return false;
  }
  const char quote = tag[quote_pos];
  const std::size_t value_begin = quote_pos + 1;
  const std::size_t value_end = tag.find(quote, value_begin);
  if (value_end == std::string::npos) {
    return false;
  }
  value = tag.substr(value_begin, value_end - value_begin);
  return true;
}

// Total number of elements of a variable: 1 for a scalar, the product of the
// dimensions for an array of any rank
std::size_t parse_variable_size(const std::string &xml,
                                            const std::string &name) {
  constexpr const char *tags[] = {"Float64",      "Float32", "Int8",   "UInt8",
                                  "Int16",        "UInt16",  "Int32",  "UInt32",
                                  "Int64",        "UInt64",  "Boolean", "String",
                                  "Binary",       "Enumeration", "Clock"};

  for (const auto *tag : tags) {
    const std::string marker = std::string("<") + tag;
    std::size_t pos = 0;
    while ((pos = xml.find(marker, pos)) != std::string::npos) {
      const std::size_t tag_end = xml.find('>', pos);
      if (tag_end == std::string::npos) {
        break;
      }
      const std::string header = xml.substr(pos, tag_end - pos + 1);
      std::string var_name;
      if (!extract_xml_attribute(header, "name", var_name) || var_name != name) {
        pos = tag_end + 1;
        continue;
      }

      std::size_t size = 1;
      if (header.size() >= 2 && header[header.size() - 2] == '/') {
        return size;
      }

      const std::string end_tag = std::string("</") + tag + ">";
      const std::size_t close_pos = xml.find(end_tag, tag_end + 1);
      if (close_pos == std::string::npos) {
        throw std::runtime_error("Malformed modelDescription.xml for variable " +
                                 name);
      }

      const std::string body = xml.substr(tag_end + 1, close_pos - tag_end - 1);
      std::size_t dim_pos = 0;
      while ((dim_pos = body.find("<Dimension", dim_pos)) != std::string::npos) {
        const std::size_t dim_end = body.find('>', dim_pos);
        if (dim_end == std::string::npos) {
          throw std::runtime_error(
              "Malformed Dimension tag in modelDescription.xml for variable " +
              name);
        }
        const std::string dim_tag =
            body.substr(dim_pos, dim_end - dim_pos + 1);
        std::string start_value;
        if (!extract_xml_attribute(dim_tag, "start", start_value)) {
          throw std::runtime_error(
              "Variable " + name +
              " has non-literal or missing array dimension; unsupported");
        }

        std::size_t consumed = 0;
        const unsigned long long len =
            std::stoull(start_value, &consumed, 10);
        if (consumed != start_value.size() || len == 0ULL) {
          throw std::runtime_error(
              "Variable " + name +
              " has invalid array dimension in modelDescription.xml");
        }
        if (len > std::numeric_limits<std::size_t>::max()) {
          throw std::runtime_error(
              "Variable " + name +
              " array dimension does not fit in host size_t");
        }
        size *= static_cast<std::size_t>(len);
        dim_pos = dim_end + 1;
      }
      return size;
    }
  }

  throw std::runtime_error("Variable not found in modelDescription.xml: " + name);
}

std::string read_model_description_xml(const std::string &fmu_path) {
  const fs::path path(fmu_path);
  if (fs::is_directory(path)) {
    return read_text_file(path / "modelDescription.xml");
  }

#if defined(_WIN32)
  const auto now =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  const fs::path temp_dir =
      fs::temp_directory_path() / ("fmu_xml_" + std::to_string(now));
  try {
    fs::create_directories(temp_dir);
    const std::string extract_cmd = "tar -xf \"" + path.string() + "\" -C \"" +
                                    temp_dir.string() + "\" modelDescription.xml";
    run_command_capture(extract_cmd);
    const std::string xml = read_text_file(temp_dir / "modelDescription.xml");
    std::error_code ec;
    fs::remove_all(temp_dir, ec);
    return xml;
  } catch (...) {
    std::error_code ec;
    fs::remove_all(temp_dir, ec);
    throw;
  }
#else
  const std::string quoted_fmu = shell_quote_posix(path.string());
  return run_command_capture("unzip -p " + quoted_fmu + " modelDescription.xml");
#endif
}

std::string parse_variable_dimensions_string(const std::string &xml,
                                            const std::string &name) {
  constexpr const char *tags[] = {"Float64",      "Float32", "Int8",   "UInt8",
                                  "Int16",        "UInt16",  "Int32",  "UInt32",
                                  "Int64",        "UInt64",  "Boolean", "String",
                                  "Binary",       "Enumeration", "Clock"};

  for (const auto *tag : tags) {
    const std::string marker = std::string("<") + tag;
    std::size_t pos = 0;
    while ((pos = xml.find(marker, pos)) != std::string::npos) {
      const std::size_t tag_end = xml.find('>', pos);
      if (tag_end == std::string::npos) {
        break;
      }
      const std::string header = xml.substr(pos, tag_end - pos + 1);
      std::string var_name;
      if (!extract_xml_attribute(header, "name", var_name) || var_name != name) {
        pos = tag_end + 1;
        continue;
      }
      if (header.size() >= 2 && header[header.size() - 2] == '/') {
        return "";
      }

      const std::string end_tag = std::string("</") + tag + ">";
      const std::size_t close_pos = xml.find(end_tag, tag_end + 1);
      if (close_pos == std::string::npos) {
        throw std::runtime_error("Malformed modelDescription.xml for variable " +
                                 name);
      }

      const std::string body = xml.substr(tag_end + 1, close_pos - tag_end - 1);
      std::vector<std::string> dims;
      std::size_t dim_pos = 0;
      while ((dim_pos = body.find("<Dimension", dim_pos)) != std::string::npos) {
        const std::size_t dim_end = body.find('>', dim_pos);
        if (dim_end == std::string::npos) {
          throw std::runtime_error(
              "Malformed Dimension tag in modelDescription.xml for variable " +
              name);
        }
        const std::string dim_tag = body.substr(dim_pos, dim_end - dim_pos + 1);
        std::string dim_value;
        if (!extract_xml_attribute(dim_tag, "start", dim_value)) {
          dim_value = "?";
        }
        dims.push_back(dim_value);
        dim_pos = dim_end + 1;
      }
      if (dims.empty()) {
        return "";
      }
      std::ostringstream ss;
      ss << "[";
      for (std::size_t i = 0; i < dims.size(); ++i) {
        if (i > 0) {
          ss << "x";
        }
        ss << dims[i];
      }
      ss << "]";
      return ss.str();
    }
  }
  return "";
}

std::string run_command_capture(const std::string &command) {
#ifdef _WIN32
  FILE *pipe = _popen(command.c_str(), "r");
#else
  FILE *pipe = popen(command.c_str(), "r");
#endif
  if (!pipe) {
    throw std::runtime_error("Failed to execute command: " + command);
  }

  std::array<char, 4096> buffer{};
  std::string output;
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    output += buffer.data();
  }

#ifdef _WIN32
  const int rc = _pclose(pipe);
#else
  const int rc = pclose(pipe);
#endif
  if (rc != 0) {
    throw std::runtime_error("Command failed (" + std::to_string(rc) + "): " +
                             command + "\n" + output);
  }
  return output;
}

std::string get_platform_dir() {
#if defined(_WIN32)
#if defined(_M_X64) || defined(__x86_64__)
  return "x86_64-windows";
#elif defined(_M_IX86) || defined(__i386__)
  return "x86-windows";
#elif defined(_M_ARM64) || defined(__aarch64__)
  return "aarch64-windows";
#else
  return "x86_64-windows";
#endif
#elif defined(__APPLE__)
#if defined(__aarch64__) || defined(__arm64__)
  return "aarch64-darwin";
#elif defined(__x86_64__)
  return "x86_64-darwin";
#else
  throw std::runtime_error("Unsupported macOS architecture for FMU binaries");
#endif
#elif defined(__linux__)
#if defined(__aarch64__)
  return "aarch64-linux";
#elif defined(__x86_64__)
  return "x86_64-linux";
#else
  throw std::runtime_error("Unsupported Linux architecture for FMU binaries");
#endif
#else
  throw std::runtime_error("Unsupported platform");
#endif
}

std::string get_binary_extension() {
#if defined(_WIN32)
  return ".dll";
#elif defined(__APPLE__)
  return ".dylib";
#else
  return ".so";
#endif
}

std::string find_model_identifier(const std::string &xml) {
  constexpr const char *tags[] = {"ModelExchange", "CoSimulation"};
  for (const auto *tag : tags) {
    const std::string marker = std::string("<") + tag;
    std::size_t pos = 0;
    while ((pos = xml.find(marker, pos)) != std::string::npos) {
      const std::size_t close = xml.find('>', pos);
      if (close == std::string::npos) {
        break;
      }
      const std::string header = xml.substr(pos, close - pos + 1);
      const std::size_t key = header.find("modelIdentifier=");
      if (key != std::string::npos) {
        const std::size_t quote_pos = key + std::string("modelIdentifier=").size();
        if (quote_pos < header.size() &&
            (header[quote_pos] == '"' || header[quote_pos] == '\'')) {
          const char q = header[quote_pos];
          const std::size_t value_begin = quote_pos + 1;
          const std::size_t value_end = header.find(q, value_begin);
          if (value_end != std::string::npos && value_end > value_begin) {
            return header.substr(value_begin, value_end - value_begin);
          }
        }
      }
      pos = close + 1;
    }
  }
  return "";
}

std::vector<std::string> list_binaries_in_directory(const fs::path &dir,
                                                    const std::string &ext) {
  std::vector<std::string> matches;
  if (!fs::exists(dir) || !fs::is_directory(dir)) {
    return matches;
  }
  for (const auto &entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (entry.path().extension() == ext) {
      matches.push_back(entry.path().string());
    }
  }
  return matches;
}

std::vector<std::string> parse_zip_listing_for_binaries(
    const std::string &listing, const std::string &platform_dir,
    const std::string &ext) {
  std::vector<std::string> entries;
  const std::string prefix = "binaries/" + platform_dir + "/";
  std::istringstream iss(listing);
  std::string line;
  while (std::getline(iss, line)) {
    line = trim_copy(line);
    if (line.rfind(prefix, 0) == 0 && line.size() >= ext.size() &&
        line.substr(line.size() - ext.size()) == ext) {
      entries.push_back(line);
    }
  }
  return entries;
}

std::vector<std::string> parse_dependency_output(const std::string &output) {
  std::vector<std::string> deps;
  std::istringstream iss(output);
  std::string line;

#if defined(_WIN32)
  while (std::getline(iss, line)) {
    line = trim_copy(line);
    if (line.empty()) {
      continue;
    }
    std::string lower = line;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const auto pos = lower.rfind(".dll");
    if (pos == std::string::npos || pos + 4 != lower.size()) {
      continue;
    }
    const auto sep = line.find_first_of(" \t");
    const std::string dep = (sep == std::string::npos) ? line : line.substr(0, sep);
    if (!dep.empty()) {
      deps.push_back(dep);
    }
  }
#elif defined(__APPLE__)
  bool first_line = true;
  while (std::getline(iss, line)) {
    line = trim_copy(line);
    if (line.empty()) {
      continue;
    }
    if (first_line) {
      first_line = false;
      continue;
    }
    const auto pos = line.find_first_of(" \t");
    const std::string dep = (pos == std::string::npos) ? line : line.substr(0, pos);
    if (!dep.empty()) {
      deps.push_back(dep);
    }
  }
#else
  while (std::getline(iss, line)) {
    line = trim_copy(line);
    if (line.empty() || line.find("statically linked") != std::string::npos) {
      continue;
    }
    std::string dep;
    const auto arrow = line.find("=>");
    if (arrow != std::string::npos) {
      dep = trim_copy(line.substr(0, arrow));
    } else {
      const auto pos = line.find_first_of(" \t");
      dep = (pos == std::string::npos) ? line : line.substr(0, pos);
    }
    if (!dep.empty()) {
      deps.push_back(dep);
    }
  }
#endif
  return deps;
}

// Read the n elements of a variable: a JSON scalar, or a flat JSON array
template <typename T>
json read_values(fmi3Status (*getter)(fmi3InstanceHandle *,
                                      const fmi3ValueReference[], size_t, T[],
                                      size_t),
                 fmi3InstanceHandle *instance, fmi3ValueReference vr,
                 std::size_t n, bool scalar, fmi3Status &status) {
  auto buffer = std::make_unique<T[]>(n); // vector<bool> has no data()
  status = getter(instance, &vr, 1, buffer.get(), n);
  if (scalar) {
    return json(buffer[0]);
  }
  json values = json::array();
  for (std::size_t i = 0; i < n; ++i) {
    values.push_back(buffer[i]);
  }
  return values;
}

// Dormand-Prince 5(4). Row 6 of DP_A is the 5th-order solution, so stage 7 is
// evaluated at the new state and is reused as stage 1 of the next step (FSAL).
constexpr double DP_C[7] = {0.0, 1.0 / 5, 3.0 / 10, 4.0 / 5, 8.0 / 9, 1.0, 1.0};
constexpr double DP_A[7][6] = {
    {},
    {1.0 / 5},
    {3.0 / 40, 9.0 / 40},
    {44.0 / 45, -56.0 / 15, 32.0 / 9},
    {19372.0 / 6561, -25360.0 / 2187, 64448.0 / 6561, -212.0 / 729},
    {9017.0 / 3168, -355.0 / 33, 46732.0 / 5247, 49.0 / 176, -5103.0 / 18656},
    {35.0 / 384, 0.0, 500.0 / 1113, 125.0 / 192, -2187.0 / 6784, 11.0 / 84}};
// 5th-order minus 4th-order weights: the error estimate needs all 7 stages
constexpr double DP_E[7] = {71.0 / 57600, 0.0, -71.0 / 16695, 71.0 / 1920,
                            -17253.0 / 339200, 22.0 / 525, -1.0 / 40};

} // namespace

// Static map initializations for enum-to-string translation
const std::unordered_map<int, std::string> FmuWrapper::_initial_kind_map = {
    {fmi3Initial::fmi3InitialExact, "exact"},
    {fmi3Initial::fmi3InitialApprox, "approx"},
    {fmi3Initial::fmi3InitialCalculated, "calculated"},
    {fmi3Initial::fmi3InitialUndefined, "undefined"}};

const std::unordered_map<fmi3Causality, std::string>
    FmuWrapper::_causality_map = {
        {fmi3Causality::fmi3CausalityStructuralParameter,
         "structuralParameter"},
        {fmi3Causality::fmi3CausalityOutput, "output"},
        {fmi3Causality::fmi3CausalityInput, "input"},
        {fmi3Causality::fmi3CausalityIndependent, "independent"},
        {fmi3Causality::fmi3CausalityCalculatedParameter,
         "calculatedParameter"},
        {fmi3Causality::fmi3CausalityParameter, "parameter"},
        {fmi3Causality::fmi3CausalityLocal, "local"}};

const std::unordered_map<fmi3DataType, std::string> FmuWrapper::_data_type_map =
    {{fmi3DataType::fmi3DataTypeFloat64, "Float64"},
     {fmi3DataType::fmi3DataTypeFloat32, "Float32"},
     {fmi3DataType::fmi3DataTypeInt8, "Int8"},
     {fmi3DataType::fmi3DataTypeUInt8, "UInt8"},
     {fmi3DataType::fmi3DataTypeInt16, "Int16"},
     {fmi3DataType::fmi3DataTypeUInt16, "UInt16"},
     {fmi3DataType::fmi3DataTypeInt32, "Int32"},
     {fmi3DataType::fmi3DataTypeUInt32, "UInt32"},
     {fmi3DataType::fmi3DataTypeInt64, "Int64"},
     {fmi3DataType::fmi3DataTypeUInt64, "UInt64"},
     {fmi3DataType::fmi3DataTypeBoolean, "Boolean"},
     {fmi3DataType::fmi3DataTypeString, "String"},
     {fmi3DataType::fmi3DataTypeBinary, "Binary"},
     {fmi3DataType::fmi3DataTypeEnumeration, "Enumeration"},
     {fmi3DataType::fmi3DataTypeClock, "Clock"}};

typedef union {
  double dbl;
  float flt;
  int8_t i8;
  uint8_t u8;
  int16_t i16;
  uint16_t u16;
  int32_t i32;
  uint32_t u32;
  int64_t i64;
  uint64_t u64;
  bool b;
} data_type_t;

// Helper methods to translate enums to strings
std::string FmuWrapper::get_initial_kind_string(fmi3Initial kind) {
  auto it = _initial_kind_map.find(kind);
  return (it != _initial_kind_map.end())
             ? it->second
             : "unknown(" + std::to_string(kind) + ")";
}

std::string FmuWrapper::get_causality_string(fmi3Causality causality) {
  auto it = _causality_map.find(causality);
  return (it != _causality_map.end())
             ? it->second
             : "unknown(" + std::to_string(causality) + ")";
}

std::string FmuWrapper::get_data_type_string(fmi3DataType dataType) {
  auto it = _data_type_map.find(dataType);
  return (it != _data_type_map.end())
             ? it->second
             : "unknown(" + std::to_string(dataType) + ")";
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

  // A constructor that throws does not run the destructor
  struct Guard {
    FmuWrapper *_wrapper;
    ~Guard() {
      if (_wrapper) {
        _wrapper->release();
      }
    }
  } guard{this};

  auto v = fmi4c_getFmiVersion(_fmu);
  if (v != 3) {
    throw std::runtime_error(std::string("Unsupported FMU version: ") +
                             std::to_string(v));
  }

  _model_description_xml = read_model_description_xml(_fmu_path);
  bool cs = _model_description_xml.find("<CoSimulation") != std::string::npos;
  bool me = _model_description_xml.find("<ModelExchange") != std::string::npos;

  if(cs){

    _type = FmuType::CoSimulation;

    size_t cs_pos = _model_description_xml.find("<CoSimulation");
    if(cs_pos != string::npos){

      size_t cs_end = _model_description_xml.find(">", cs_pos);
      string cs_tag = _model_description_xml.substr(cs_pos, cs_end - cs_pos + 1);

      // FMI 3.0: without this attribute the FMU needs a fixed step
      string var_step, step_string;
      const bool variable_step =
          extract_xml_attribute(
              cs_tag, "canHandleVariableCommunicationStepSize", var_step) &&
          (var_step == "true" || var_step == "1");
      if (extract_xml_attribute(cs_tag, "fixedInternalStepSize", step_string)) {
        _step_size = std::stod(step_string);
      }
      // Without a declared internal step there is nothing to enforce
      _fixed_step = !variable_step && _step_size > 0.0;
    }

    _instance = fmi3_instantiateCoSimulation(
      _fmu,
      fmi3False, // visible
      fmi3False, // loggingOn
      fmi3False, // eventModeUsed
      fmi3False, // earlyReturnAllowed
      nullptr,   // requiredIntermediateVariables
      0,         // nRequiredIntermediateVariables
      nullptr,   // instanceEnvironment
      nullptr,   // logMessage
      nullptr    // intermediateUpdate
    );

  } else if(me){

    _type = FmuType::ModelExchange;
    _instance = fmi3_instantiateModelExchange(_fmu, false, false, nullptr, nullptr);

  } else{

    throw std::runtime_error("FMU Type: Unknown");
  }

  if (!_instance) {
    fmi4c_freeFmu(_fmu);
    _fmu = nullptr;
    throw std::runtime_error("Failed to instantiate FMU: " + instance_name);
  }

  int vnum = fmi3_getNumberOfVariables(_fmu);
  for (int i = 0; i < vnum; ++i) {
    auto v = fmi3_getVariableByIndex(_fmu, i + 1);
    if (fmi3_getVariableCausality(v) == fmi3Causality::fmi3CausalityOutput) {
      _out_vars.push_back(fmi3_getVariableName(v));
    } else if (fmi3_getVariableCausality(v) ==
               fmi3Causality::fmi3CausalityInput) {
      _in_vars.push_back(fmi3_getVariableName(v));
    } else if (fmi3_getVariableCausality(v) ==
               fmi3Causality::fmi3CausalityParameter) {
      _param_vars.push_back(fmi3_getVariableName(v));
    } else if (fmi3_getVariableCausality(v) ==
               fmi3Causality::fmi3CausalityIndependent) {
      _indep_vars.push_back(fmi3_getVariableName(v));
    }
  }
  guard._wrapper = nullptr;
}

void FmuWrapper::initialize() {
  if (!_instance) {
    throw std::runtime_error("FMU instance not created");
  }
  check_status(fmi3_enterInitializationMode(_instance,
                                            false,       // toleranceDefined
                                            0.0,         // tolerance
                                            _start_time, // startTime
                                            false,       // stopTimeDefined
                                            0.0),        // stopTime
               "enterInitializationMode");
  check_status(fmi3_exitInitializationMode(_instance),
               "exitInitializationMode");
  _current_time = _start_time;
  _h_last = 0.0;
  _next_event_time_defined = fmi3False;
  _initialized = true;
  if (_type == FmuType::ModelExchange) {
    // A Model Exchange FMU leaves initialization in Event Mode, and stays
    // parked there between do_step() calls
    handle_events();
  }
}

FmuWrapper::~FmuWrapper() { release(); }

void FmuWrapper::release() {
  if (_instance) {
    if (_initialized) {
      fmi3_terminate(_instance);
    }
    fmi3_freeInstance(_instance);
    _instance = nullptr;
  }
  if (_fmu) {
    fmi4c_freeFmu(_fmu);
    _fmu = nullptr;
  }
}

std::string FmuWrapper::model_name() {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  return fmi3_modelName(_instance->fmu);
}

void FmuWrapper::reset() {
  if (!_instance) {
    throw std::runtime_error("FMU instance not created");
  }
  check_status(fmi3_reset(_instance), "Resetting the model");
  // Back to the Instantiated state: set parameters, then call initialize()
  _initialized = false;
  _current_time = _start_time;

}

void FmuWrapper::do_step(double dt) {

  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }

  if (_type == FmuType::CoSimulation) { // CO-SIMULATION

    fmi3Boolean eventHandlingNeeded = fmi3False;
    fmi3Boolean terminateSimulation = fmi3False;
    fmi3Boolean earlyReturn = fmi3False;
    fmi3Float64 lastSuccessfulTime = _current_time;

    // exported solver within FMU
    fmi3Status status = fmi3_doStep(_instance,
                                    _current_time,
                                    dt,
                                    fmi3True, // noSetFMUStatePriorToCurrentPoint
                                    &eventHandlingNeeded,
                                    &terminateSimulation,
                                    &earlyReturn,
                                    &lastSuccessfulTime);
                                    
    check_status(status, "doStep");

    if (terminateSimulation) {
      throw std::runtime_error("FMU requested termination during doStep");
    }

    _current_time += dt;

    if (eventHandlingNeeded) {
        fmi3_enterEventMode(_instance);
        handle_events();
        fmi3_enterStepMode(_instance);
    }

  } else if (_type == FmuType::ModelExchange) { // MODEL EXCHANGE

    const double rel_tol = _solver_params._rel_tol;
    const double abs_tol = _solver_params._abs_tol;
    const double hmin = _solver_params._hmin;
    const double tend = _current_time + dt;

    size_t n = 0, nz = 0;
    check_status(fmi3_getNumberOfContinuousStates(_instance, &n),
                 "getNumberOfContinuousStates");
    check_status(fmi3_getNumberOfEventIndicators(_instance, &nz),
                 "getNumberOfEventIndicators");

    // The FMU is parked in Event Mode between calls: inputs set since the last
    // call are processed here, then integration runs in Continuous-Time Mode
    handle_events();
    check_status(fmi3_enterContinuousTimeMode(_instance),
                 "enterContinuousTimeMode");

    std::vector<fmi3Float64> y(n), ynew(n), ytemp(n), scratch(n);
    std::vector<fmi3Float64> z(nz), znew(nz);
    std::array<std::vector<fmi3Float64>, 7> k;
    for (auto &ki : k) {
      ki.resize(n);
    }
    if (n) {
      check_status(fmi3_getContinuousStates(_instance, y.data(), n),
                   "getContinuousStates");
    }
    if (nz) {
      check_status(fmi3_getEventIndicators(_instance, z.data(), nz),
                   "getEventIndicators");
    }

    // dx = f(ts, x)
    auto f = [&](double ts, const std::vector<fmi3Float64> &x,
                 std::vector<fmi3Float64> &dx) {
      check_status(fmi3_setTime(_instance, ts), "setTime");
      if (!n) {
        return;
      }
      check_status(fmi3_setContinuousStates(_instance, x.data(), n),
                   "setContinuousStates");
      // Simulink FMUs refresh their block outputs lazily, and only after
      // computing the derivatives: reading the states first forces the refresh
      check_status(fmi3_getContinuousStates(_instance, scratch.data(), n),
                   "getContinuousStates");
      check_status(fmi3_getContinuousStateDerivatives(_instance, dx.data(), n),
                   "getContinuousStateDerivatives");
    };

    double t = _current_time;
    double h = _h_last > 0.0 ? _h_last : dt;
    bool k1_valid = false; // inputs may have changed since the last call

    while (t < tend) {
      if (h < hmin) {
        throw std::runtime_error("RK45: step size below hmin at t=" +
                                 std::to_string(t));
      }
      // Never step across a scheduled time event or past tend
      const double tstop = (_next_event_time_defined && _next_event_time > t &&
                            _next_event_time < tend)
                               ? _next_event_time
                               : tend;
      const double h_ctrl = h; // step wanted by the controller
      const bool truncated = t + 1.01 * h >= tstop;
      if (truncated) {
        h = tstop - t;
      }

      if (!k1_valid) {
        f(t, y, k[0]);
        k1_valid = true;
      }
      for (int s = 1; s < 7; ++s) {
        auto &x = (s == 6) ? ynew : ytemp;
        for (size_t i = 0; i < n; ++i) {
          double sum = 0.0;
          for (int j = 0; j < s; ++j) {
            sum += DP_A[s][j] * k[j][i];
          }
          x[i] = y[i] + h * sum;
        }
        f(t + DP_C[s] * h, x, k[s]);
      }

      double err = 0.0;
      for (size_t i = 0; i < n; ++i) {
        double e = 0.0;
        for (int j = 0; j < 7; ++j) {
          e += DP_E[j] * k[j][i];
        }
        const double r =
            std::abs(h * e) /
            (abs_tol + rel_tol * std::max(std::abs(y[i]), std::abs(ynew[i])));
        if (!std::isfinite(r)) { // std::max would silently drop a NaN
          err = std::numeric_limits<double>::infinity();
          break;
        }
        err = std::max(err, r);
      }

      if (err <= 1.0) {
        // Step accepted; the FMU already holds ynew from stage 7
        t = truncated ? tstop : t + h;
        y.swap(ynew);
        k[0].swap(k[6]);
        _current_time = t;
        check_status(fmi3_setTime(_instance, t), "setTime");

        fmi3Boolean enter_event = fmi3False, terminate = fmi3False;
        check_status(fmi3_completedIntegratorStep(_instance, fmi3True,
                                                  &enter_event, &terminate),
                     "completedIntegratorStep");
        if (terminate) {
          throw std::runtime_error("FMU requested termination during step");
        }
        bool event =
            enter_event || (_next_event_time_defined && t >= _next_event_time);
        if (nz) {
          check_status(fmi3_getEventIndicators(_instance, znew.data(), nz),
                       "getEventIndicators");
          for (size_t j = 0; j < nz; ++j) {
            event = event || ((z[j] > 0.0) != (znew[j] > 0.0));
          }
          z.swap(znew);
        }
        if (event) {
          // shortcut: a state event is handled at the end of the step where
          // it is detected, not located; add root finding if an FMU needs it
          check_status(fmi3_enterEventMode(_instance), "enterEventMode");
          handle_events();
          check_status(fmi3_enterContinuousTimeMode(_instance),
                       "enterContinuousTimeMode");
          if (n) {
            check_status(fmi3_getContinuousStates(_instance, y.data(), n),
                         "getContinuousStates");
          }
          if (nz) {
            check_status(fmi3_getEventIndicators(_instance, z.data(), nz),
                         "getEventIndicators");
          }
          k1_valid = false;
        }
        // A step shortened to hit tstop must not shrink the controller's step
        h = std::max(h * std::clamp(0.9 * std::pow(err, -0.2), 0.2, 5.0),
                     truncated ? h_ctrl : 0.0);
      } else {
        // Step rejected: k[0] is still f(t, y), the retry sets the stage states
        h *= std::clamp(0.9 * std::pow(err, -0.2), 0.2, 1.0);
      }
    }

    _h_last = std::min(h, dt);
    _current_time = tend;
    check_status(fmi3_enterEventMode(_instance), "enterEventMode");
    handle_events();
  }
}

void FmuWrapper::handle_events() {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  fmi3Boolean discrete_states_need_update = fmi3True;
  fmi3Boolean terminate = fmi3False;
  fmi3Boolean nominals_changed = fmi3False;
  fmi3Boolean values_changed = fmi3False;

  while (discrete_states_need_update) {
    check_status(fmi3_updateDiscreteStates(_instance,
                                           &discrete_states_need_update,
                                           &terminate, &nominals_changed,
                                           &values_changed,
                                           &_next_event_time_defined,
                                           &_next_event_time),
                 "updateDiscreteStates");
    if (terminate) {
      throw std::runtime_error(
          "FMU requested termination during event handling");
    }
  }
  if (_type == FmuType::ModelExchange) {
    // Simulink's model_update() overwrites the model time with zero
    check_status(fmi3_setTime(_instance, _current_time), "setTime");
  }
}

void FmuWrapper::set_real(const std::string &name, double value) {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  require_scalar(name);

  fmi3Status status = fmi3_setFloat64(_instance, &vr, 1, &value, 1);
  check_status(status, std::string("setFloat64 for variable ") + name);
}

void FmuWrapper::set_bool(const std::string &name, bool value) {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  require_scalar(name);
  const fmi3Boolean fmi_value = value;

  fmi3Status status = fmi3_setBoolean(_instance, &vr, 1, &fmi_value, 1);
  check_status(status, std::string("setBoolean for variable ") + name);
}

void FmuWrapper::set_real(const std::string &name,
                          const std::vector<double> &values) {
  if (!_instance) {
    throw std::runtime_error("FMU instance not created");
  }
  require_float64(name);
  const std::size_t expected = resolve_var_size(name);
  if (values.size() != expected) {
    throw std::runtime_error("Size mismatch for variable " + name +
                             ": expected " + std::to_string(expected) +
                             ", got " + std::to_string(values.size()));
  }
  if (values.empty()) {
    throw std::runtime_error("Cannot set empty array for variable " + name);
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  fmi3Status status =
      fmi3_setFloat64(_instance, &vr, 1, values.data(), values.size());
  check_status(status, std::string("setFloat64 array for variable ") + name);
}

double FmuWrapper::get_real(const std::string &name) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  require_scalar(name);
  fmi3Float64 value = 0.0;

  fmi3Status status = fmi3_getFloat64(_instance, &vr, 1, &value, 1);
  check_status(status, std::string("getFloat64 for variable ") + name);

  return value;
}

bool FmuWrapper::get_bool(const std::string &name) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  require_scalar(name);
  fmi3Boolean value = fmi3False;

  fmi3Status status = fmi3_getBoolean(_instance, &vr, 1, &value, 1);
  check_status(status, std::string("getBoolean for variable ") + name);

  return value;
}

void FmuWrapper::get_real(const std::string &name,
                          std::vector<double> &values) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  require_float64(name);
  const std::size_t expected = resolve_var_size(name);
  values.resize(expected);
  if (values.empty()) {
    throw std::runtime_error("Cannot get empty array for variable " + name);
  }

  fmi3ValueReference vr = resolve_var_ref(name);
  fmi3Status status =
      fmi3_getFloat64(_instance, &vr, 1, values.data(), values.size());
  check_status(status, std::string("getFloat64 array for variable ") + name);
}

void FmuWrapper::list_variables(ostream &s) {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  auto &fmu = _instance->fmu;
  int vnum = fmi3_getNumberOfVariables(fmu);
  string name = fmi3_modelName(fmu);
  const int fw = 15;
  const int tw = 18;
  s << style::bold << "Variables:" << endl
    << setw(fw) << "Name"
    << setw(fw) << "Description"
    << setw(tw) << "Data type"
    << setw(fw) << "Causality"
    << setw(fw) << "Initial"
    << style::reset << endl;
  for (int i = 0; i < vnum; ++i) {
    auto v = fmi3_getVariableByIndex(fmu, i + 1);
    const std::string var_name = fmi3_getVariableName(v);
    auto c = fmi3_getVariableCausality(v);
    if (c > fmi3Causality::fmi3CausalityCalculatedParameter)
      continue;
    std::string dtype = get_data_type_string(fmi3_getVariableDataType(v));
    const char *description = fmi3_getVariableDescription(v);
    const std::string dims = resolve_var_dimensions(var_name);
    if (!dims.empty()) {
      // Convert "[3x2]" to "(3x2)" for compact table output.
      dtype += "(" + dims.substr(1, dims.size() - 2) + ")";
    }

    s << setw(fw) << var_name
      << setw(fw) << (description ? description : "")
      << setw(tw) << dtype
      << setw(fw) << get_causality_string(c)
      << setw(fw) << get_initial_kind_string(fmi3_getVariableInitial(v))
      << endl;
  }
}

size_t FmuWrapper::get_outputs(std::map<std::string, double> &outs) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  size_t n = 0;
  for (auto const &name : _out_vars) {
    outs[name] = get_real(name);
    n++;
  }
  return n;
}

size_t FmuWrapper::get_inputs(std::map<std::string, double> &ins) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  size_t n = 0;
  for (auto const &name : _in_vars) {
    ins[name] = get_real(name);
    n++;
  }
  return n;
}

size_t FmuWrapper::get_params(std::map<std::string, double> &params) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  size_t n = 0;
  for (auto const &name : _param_vars) {
    params[name] = get_real(name);
    n++;
  }
  return n;
}

size_t FmuWrapper::get_indep(std::map<std::string, double> &indep) const {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  size_t n = 0;
  for (auto const &name : _indep_vars) {
    indep[name] = get_real(name);
    n++;
  }
  return n;
}

json FmuWrapper::value(string const &name) {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  const fmi3ValueReference vref = resolve_var_ref(name);
  const std::size_t n = resolve_var_size(name);
  const bool scalar = resolve_var_dimensions(name).empty();
  const fmi3DataType type = var_type(name);
  fmi3Status status = fmi3OK;
  json jval;
  switch (type) {
  case fmi3DataType::fmi3DataTypeFloat64:
    jval = read_values(fmi3_getFloat64, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeFloat32:
    jval = read_values(fmi3_getFloat32, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeInt8:
    jval = read_values(fmi3_getInt8, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeUInt8:
    jval = read_values(fmi3_getUInt8, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeInt16:
    jval = read_values(fmi3_getInt16, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeUInt16:
    jval = read_values(fmi3_getUInt16, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeInt32:
    jval = read_values(fmi3_getInt32, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeUInt32:
    jval = read_values(fmi3_getUInt32, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeInt64:
  case fmi3DataType::fmi3DataTypeEnumeration: // enumerations are read as Int64
    jval = read_values(fmi3_getInt64, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeUInt64:
    jval = read_values(fmi3_getUInt64, _instance, vref, n, scalar, status);
    break;
  case fmi3DataType::fmi3DataTypeBoolean:
    jval = read_values(fmi3_getBoolean, _instance, vref, n, scalar, status);
    break;
  default:
    throw std::runtime_error("Unsupported value data type " +
                             get_data_type_string(type));
  }
  check_status(status,
               "get" + get_data_type_string(type) + " for variable " + name);
  return jval;
}

void FmuWrapper::set(const std::string &name, const json &value) {
  resolve_var_ref(name); // throws if the variable does not exist
  const fmi3DataType type = var_type(name);
  if (type == fmi3DataType::fmi3DataTypeFloat64 && value.is_array()) {
    set_real(name, value.get<std::vector<double>>());
  } else if (type == fmi3DataType::fmi3DataTypeFloat64 && value.is_number()) {
    set_real(name, value.get<double>());
  } else if (type == fmi3DataType::fmi3DataTypeBoolean && value.is_boolean()) {
    set_bool(name, value.get<bool>());
  } else {
    // shortcut: only Float64 and Boolean variables can be written; add the
    // integer types when an FMU needs them
    throw std::runtime_error("Cannot set " + name + " (" +
                             get_data_type_string(type) + ") from a JSON " +
                             value.type_name());
  }
}

void FmuWrapper::get_status(json &status) {
  if (!_instance || !_initialized) {
    throw std::runtime_error("FMU instance not initialized");
  }
  status.clear();
  auto &fmu = _instance->fmu;
  int vnum = fmi3_getNumberOfVariables(fmu);
  string name;
  for (int i = 0; i < vnum; ++i) {
    auto v = fmi3_getVariableByIndex(fmu, i + 1);
    name = fmi3_getVariableName(v);
    auto causality = fmi3_getVariableCausality(v);
    if (causality == fmi3Causality::fmi3CausalityLocal ||
        causality == fmi3Causality::fmi3CausalityCalculatedParameter) {
      continue;
    }
    const auto type = fmi3_getVariableDataType(v);
    if (type == fmi3DataType::fmi3DataTypeString ||
        type == fmi3DataType::fmi3DataTypeBinary ||
        type == fmi3DataType::fmi3DataTypeClock) {
      continue; // shortcut: not published, add when an FMU needs them
    }
    status[get_causality_string(causality)][name] = value(name);
  }
}

std::vector<std::string> FmuWrapper::get_input_names() const {
  return _in_vars;
}

std::vector<std::string> FmuWrapper::get_param_names() const {
  return _param_vars;
}

std::vector<std::string> FmuWrapper::get_output_names() const {
  return _out_vars;
}

std::vector<std::string> FmuWrapper::get_indep_names() const {
  return _indep_vars;
}

std::vector<std::string> FmuWrapper::get_binary_dependencies() const {
  if (_fmu_path.empty()) {
    throw std::runtime_error("FMU path is empty");
  }

  const fs::path fmu_path(_fmu_path);
  const std::string platform_dir = get_platform_dir();
  const std::string ext = get_binary_extension();

  fs::path binary_path;
  fs::path cleanup_dir;
  std::string inspect_output;
  try {
    if (fs::is_directory(fmu_path)) {
      const fs::path model_xml = fmu_path / "modelDescription.xml";
      const std::string model_id =
          find_model_identifier(read_text_file(model_xml));
      const fs::path bin_dir = fmu_path / "binaries" / platform_dir;
      auto binaries = list_binaries_in_directory(bin_dir, ext);

      if (binaries.empty()) {
        throw std::runtime_error("No platform binary found in " +
                                 bin_dir.string());
      }

      if (!model_id.empty()) {
        const fs::path expected = bin_dir / (model_id + ext);
        if (fs::exists(expected)) {
          binary_path = expected;
        }
      }
      if (binary_path.empty()) {
        binary_path = binaries.front();
      }
    } else {
#if defined(_WIN32)
      const auto now = std::chrono::high_resolution_clock::now()
                           .time_since_epoch()
                           .count();
      cleanup_dir =
          fs::temp_directory_path() / ("fmu_dep_" + std::to_string(now));
      fs::create_directories(cleanup_dir);

      const std::string extract_cmd =
          "tar -xf \"" + fmu_path.string() + "\" -C \"" +
          cleanup_dir.string() + "\"";
      run_command_capture(extract_cmd);

      const fs::path model_xml = cleanup_dir / "modelDescription.xml";
      const std::string model_id =
          find_model_identifier(read_text_file(model_xml));
      const fs::path bin_dir = cleanup_dir / "binaries" / platform_dir;
      auto binaries = list_binaries_in_directory(bin_dir, ext);
      if (binaries.empty()) {
        throw std::runtime_error("No platform binary found inside FMU for " +
                                 platform_dir);
      }

      if (!model_id.empty()) {
        const fs::path expected = bin_dir / (model_id + ext);
        if (fs::exists(expected)) {
          binary_path = expected;
        }
      }
      if (binary_path.empty()) {
        binary_path = binaries.front();
      }
#else
      const std::string quoted_fmu = shell_quote_posix(fmu_path.string());
      const std::string model_xml =
          run_command_capture("unzip -p " + quoted_fmu + " modelDescription.xml");
      const std::string model_id = find_model_identifier(model_xml);

      auto binaries = parse_zip_listing_for_binaries(
          run_command_capture("unzip -Z1 " + quoted_fmu), platform_dir, ext);
      if (binaries.empty()) {
        throw std::runtime_error("No platform binary found inside FMU for " +
                                 platform_dir);
      }

      std::string selected_entry = binaries.front();
      if (!model_id.empty()) {
        const std::string expected =
            "binaries/" + platform_dir + "/" + model_id + ext;
        auto it = std::find(binaries.begin(), binaries.end(), expected);
        if (it != binaries.end()) {
          selected_entry = *it;
        }
      }

      const auto now = std::chrono::high_resolution_clock::now()
                           .time_since_epoch()
                           .count();
      cleanup_dir =
          fs::temp_directory_path() / ("fmu_dep_" + std::to_string(now));
      fs::create_directories(cleanup_dir);

      const std::string quoted_entry = shell_quote_posix(selected_entry);
      const std::string quoted_tmp = shell_quote_posix(cleanup_dir.string());
      run_command_capture("unzip -qq -o " + quoted_fmu + " " + quoted_entry +
                          " -d " + quoted_tmp);
      binary_path = cleanup_dir / selected_entry;
#endif
    }

#if defined(_WIN32)
    inspect_output =
        run_command_capture("dumpbin /DEPENDENTS \"" + binary_path.string() + "\"");
#elif defined(__APPLE__)
    inspect_output =
        run_command_capture("otool -L " + shell_quote_posix(binary_path.string()));
#else
    inspect_output =
        run_command_capture("ldd " + shell_quote_posix(binary_path.string()));
#endif
  } catch (runtime_error &e) {
    if (!cleanup_dir.empty()) {
      std::error_code ec;
      fs::remove_all(cleanup_dir, ec);
    }
    cout << "Error: " << e.what() << endl;
    throw;
  }

  if (!cleanup_dir.empty()) {
    std::error_code ec;
    fs::remove_all(cleanup_dir, ec);
  }

  return parse_dependency_output(inspect_output);
}

fmi3ValueReference FmuWrapper::resolve_var_ref(const std::string &name) const {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }
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

size_t FmuWrapper::resolve_var_size(const std::string &name) const {
  if (!_instance) {
    throw std::runtime_error("FMU instance not created");
  }

  const auto cached = _var_size_cache.find(name);
  if (cached != _var_size_cache.end()) {
    return cached->second;
  }

  const std::size_t size = parse_variable_size(_model_description_xml, name);
  _var_size_cache[name] = size;
  return size;
}

fmi3DataType FmuWrapper::var_type(const std::string &name) const {
  resolve_var_ref(name); // throws if the variable does not exist
  return fmi3_getVariableDataType(fmi3_getVariableByName(_fmu, name.c_str()));
}

void FmuWrapper::require_scalar(const std::string &name) const {
  const std::size_t n = resolve_var_size(name);
  if (n != 1) {
    throw std::runtime_error("Variable " + name + " is an array of " +
                             std::to_string(n) + " elements");
  }
}

void FmuWrapper::require_float64(const std::string &name) const {
  if (var_type(name) != fmi3DataType::fmi3DataTypeFloat64) {
    throw std::runtime_error("Variable " + name + " is not Float64");
  }
}

std::string FmuWrapper::resolve_var_dimensions(const std::string &name) const {
  if (!_instance) {
    throw std::runtime_error("FMU instance not created");
  }

  const auto cached = _var_dimensions_cache.find(name);
  if (cached != _var_dimensions_cache.end()) {
    return cached->second;
  }

  if (_model_description_xml.empty()) {
    _model_description_xml = read_model_description_xml(_fmu_path);
  }

  const std::string dims =
      parse_variable_dimensions_string(_model_description_xml, name);
  _var_dimensions_cache[name] = dims;
  return dims;
}

void FmuWrapper::check_status(fmi3Status status,
                              const std::string &context) const {
  if (!_instance) {
    throw std::runtime_error("FMU instance not initialized");
  }
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
