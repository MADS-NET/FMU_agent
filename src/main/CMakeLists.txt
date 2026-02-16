#   _____                     _        _     _           
#  | ____|_  _____  ___ _   _| |_ __ _| |__ | | ___  ___ 
#  |  _| \ \/ / _ \/ __| | | | __/ _` | '_ \| |/ _ \/ __|
#  | |___ >  <  __/ (__| |_| | || (_| | |_) | |  __/\__ \
#  |_____/_/\_\___|\___|\__,_|\__\__,_|_.__/|_|\___||___/
#  

# Test
add_executable(test test.cpp)
target_link_libraries(test PRIVATE fmiinterface)

# Agent
add_executable(fmu_agent fmu_agent.cpp)
target_link_libraries(fmu_agent PRIVATE fmiinterface cxxopts mads-lib)