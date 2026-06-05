#   _____ __  __ ___      _                    _   
#  |  ___|  \/  |_ _|    / \   __ _  ___ _ __ | |_ 
#  | |_  | |\/| || |    / _ \ / _` |/ _ \ '_ \| __|
#  |  _| | |  | || |   / ___ \ (_| |  __/ | | | |_ 
#  |_|   |_|  |_|___| /_/   \_\__, |\___|_| |_|\__|
#                             |___/                
cmake_minimum_required(VERSION 3.28)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(FMU_agent VERSION 1.0.0 LANGUAGES C CXX)
# set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

# PROJECT SETTINGS #############################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
  add_compile_options(-fPIC)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Get MADS prefix
execute_process(
  COMMAND mads -p
  OUTPUT_VARIABLE MADS_PREFIX_RAW
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(TO_CMAKE_PATH "${MADS_PREFIX_RAW}" MADS_PREFIX)
include_directories(${MADS_PREFIX}/include)
link_directories(${MADS_PREFIX}/lib)
set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${MADS_PREFIX})

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
  # No warning on FetchContent_Populate
  cmake_policy(SET CMP0169 OLD)
endif()

option(MADS_INSTALL_AGENT "Install the FMU agent" OFF)
option(MADS_BUILD_FMU "Build the sample FMUs and enable the install_fmu target" OFF)

message(STATUS "MADS_PREFIX: ${MADS_PREFIX}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}") 
message(STATUS "mads-fmu agent will be installed: ${MADS_INSTALL_AGENT}")
message(STATUS "Example FMUs will be compiled and install_fmu will be available: ${MADS_BUILD_FMU}")

#   ____                            _                 _           
#  |  _ \  ___ _ __   ___ _ __   __| | ___ _ __   ___(_) ___  ___ 
#  | | | |/ _ \ '_ \ / _ \ '_ \ / _` |/ _ \ '_ \ / __| |/ _ \/ __|
#  | |_| |  __/ |_) |  __/ | | | (_| |  __/ | | | (__| |  __/\__ \
#  |____/ \___| .__/ \___|_| |_|\__,_|\___|_| |_|\___|_|\___||___/
#             |_|                                                 

include(FetchContent)
FetchContent_Declare(cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.1.1
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

set(FMI4C_BUILD_TESTS ON CACHE BOOL "" FORCE)
set(FMI4C_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(FMI4C_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FMI4C_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    fmi4c
    GIT_REPOSITORY https://github.com/pbosetti/fmi4c.git
    GIT_TAG        HEAD   # or a commit hash for reproducibility
    GIT_SHALLOW    TRUE
    EXCLUDE_FROM_ALL
)

# Disabilita le install() della dependency
set(_old_skip ${CMAKE_SKIP_INSTALL_RULES})
set(CMAKE_SKIP_INSTALL_RULES ON)
FetchContent_Populate(cxxopts)
add_subdirectory(${cxxopts_SOURCE_DIR} ${cxxopts_BINARY_DIR} EXCLUDE_FROM_ALL)
FetchContent_Populate(fmi4c)
add_subdirectory(${fmi4c_SOURCE_DIR} ${fmi4c_BINARY_DIR} EXCLUDE_FROM_ALL)
set(CMAKE_SKIP_INSTALL_RULES ${_old_skip})

#   ____        _ _     _ 
#  | __ ) _   _(_) | __| |
#  |  _ \| | | | | |/ _` |
#  | |_) | |_| | | | (_| |
#  |____/ \__,_|_|_|\__,_|
#                       
#

# Agent is created in the src/main directory
file(GLOB LIB_SOURCES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")
add_subdirectory(${SRC_DIR}/main)

# Static library for C+ wrapper used by the agent
add_library(fmiinterface STATIC ${LIB_SOURCES})
target_link_libraries(fmiinterface fmi4c)


#   _____ __  __ _   _     
#  |  ___|  \/  | | | |___ 
#  | |_  | |\/| | | | / __|
#  |  _| | |  | | |_| \__ \
#  |_|   |_|  |_|\___/|___/
#
# FMUs are created in the models directory
if(MADS_BUILD_FMU)
  add_subdirectory(models)
endif()


# PACKAGE #######################################################################
string(TOLOWER "${CMAKE_SYSTEM_NAME}" PACKAGE_OS_NAME)
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
  list(LENGTH CMAKE_OSX_ARCHITECTURES PACKAGE_OSX_ARCHITECTURES_COUNT)
  if(PACKAGE_OSX_ARCHITECTURES_COUNT GREATER 1)
    set(PACKAGE_ARCH_NAME "universal")
  else()
    set(PACKAGE_ARCH_NAME "${CMAKE_OSX_ARCHITECTURES}")
  endif()
else()
  set(PACKAGE_ARCH_NAME "${CMAKE_SYSTEM_PROCESSOR}")
endif()
if(PACKAGE_ARCH_NAME STREQUAL "")
  set(PACKAGE_ARCH_NAME "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()
if(PACKAGE_ARCH_NAME STREQUAL "")
  set(PACKAGE_ARCH_NAME "unknown")
endif()
string(TOLOWER "${PACKAGE_ARCH_NAME}" PACKAGE_ARCH_NAME)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "MADS")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_OS_NAME}-${PACKAGE_ARCH_NAME}")
include(CPack)
