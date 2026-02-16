#   _____ __  __ ___      _                    _   
#  |  ___|  \/  |_ _|    / \   __ _  ___ _ __ | |_ 
#  | |_  | |\/| || |    / _ \ / _` |/ _ \ '_ \| __|
#  |  _| | |  | || |   / ___ \ (_| |  __/ | | | |_ 
#  |_|   |_|  |_|___| /_/   \_\__, |\___|_| |_|\__|
#                             |___/                
cmake_minimum_required(VERSION 3.20)
project(FMU_agent VERSION 1.0.0 LANGUAGES C CXX)
# set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

# PROJECT SETTINGS #############################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 19)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# Get MADS prefix
execute_process(
  COMMAND mads -p
  OUTPUT_VARIABLE MADS_PREFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "MADS_PREFIX: ${MADS_PREFIX}")
include_directories(${MADS_PREFIX}/include)
link_directories(${MADS_PREFIX}/lib)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${MADS_PREFIX})
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}") 


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
)

set(FMI4C_BUILD_TESTS ON CACHE BOOL "" FORCE)
set(FMI4C_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(FMI4C_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    fmi4c
    GIT_REPOSITORY https://github.com/pbosetti/fmi4c.git
    GIT_TAG        HEAD   # or a commit hash for reproducibility
    GIT_SHALLOW    TRUE
)

set(BUILD_TESTING OFF CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
  GIT_SHALLOW    TRUE
)


FetchContent_Declare(rang
  GIT_REPOSITORY https://github.com/pbosetti/rang.git
  GIT_TAG        HEAD
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(cxxopts fmi4c json rang)


#   ____        _ _     _ 
#  | __ ) _   _(_) | __| |
#  |  _ \| | | | | |/ _` |
#  | |_) | |_| | | | (_| |
#  |____/ \__,_|_|_|\__,_|
#                       

file(GLOB LIB_SOURCES "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")
add_subdirectory(${SRC_DIR}/main)

add_library(fmiinterface STATIC ${LIB_SOURCES})
target_link_libraries(fmiinterface fmi4c)


#   ___           _        _ _ 
#  |_ _|_ __  ___| |_ __ _| | |
#   | || '_ \/ __| __/ _` | | |
#   | || | | \__ \ || (_| | | |
#  |___|_| |_|___/\__\__,_|_|_|
#                             
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${USR_DIR})
endif()
install(TARGETS ${TARGET_LIST}
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
)