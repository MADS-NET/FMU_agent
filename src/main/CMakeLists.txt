#   _____                     _        _     _           
#  | ____|_  _____  ___ _   _| |_ __ _| |__ | | ___  ___ 
#  |  _| \ \/ / _ \/ __| | | | __/ _` | '_ \| |/ _ \/ __|
#  | |___ >  <  __/ (__| |_| | || (_| | |_) | |  __/\__ \
#  |_____/_/\_\___|\___|\__,_|\__\__,_|_.__/|_|\___||___/
#  
if (APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
  # include_directories(/opt/homebrew/include)
  # link_directories(/opt/homebrew/lib)
else()
  set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib;/usr/local/lib")
endif()

# Test
add_executable(test test.cpp)
target_link_libraries(test PRIVATE fmiinterface)

# Agent
add_executable(mads-fmu fmu_agent.cpp)
target_link_libraries(mads-fmu PRIVATE fmiinterface cxxopts mads-lib)


install(TARGETS mads-fmu
  RUNTIME DESTINATION bin
)