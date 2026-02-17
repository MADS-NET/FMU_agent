#   _____ __  __ _   _                             _ _           
#  |  ___|  \/  | | | |   ___ ___  _ __ ___  _ __ (_) | ___ _ __ 
#  | |_  | |\/| | | | |  / __/ _ \| '_ ` _ \| '_ \| | |/ _ \ '__|
#  |  _| | |  | | |_| | | (_| (_) | | | | | | |_) | | |  __/ |   
#  |_|   |_|  |_|\___/   \___\___/|_| |_| |_| .__/|_|_|\___|_|   
#                                           |_|                  
# This is for compiling .fmu units from source file.

# Define architecture-OS pair for FMI3 compatibility
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(ARCH_PAIR "aarch64-darwin")
  else()
    set(ARCH_PAIR "x86_64-darwin")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(ARCH_PAIR "aarch64-linux")
  else()
    set(ARCH_PAIR "x86_64-linux")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(ARCH_PAIR "x86_64-win64")
  else()
    set(ARCH_PAIR "aarch64-win64")
  endif()
else()
  message(WARNING "Unknown platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
  set(ARCH_PAIR "unknown")
endif()
message(STATUS "FMI3 Architecture pair: ${ARCH_PAIR}")

# Download FMI3 headers
set(FMI_PATH ${CMAKE_CURRENT_BINARY_DIR}/FMI-Standard)
if(NOT EXISTS ${FMI_PATH})
  message(STATUS "Downloading FMI-Standard package to ${FMI_PATH}...")
  file(DOWNLOAD https://github.com/modelica/fmi-standard/releases/download/v3.0.2/FMI-Standard-3.0.2.zip ${FMI_PATH}/FMI-Standard-3.0.2.zip)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${FMI_PATH}/FMI-Standard-3.0.2.zip WORKING_DIRECTORY ${FMI_PATH})
  message(STATUS "done.")
endif()
include_directories(${FMI_PATH}/headers)
if(!WIN32)
  add_compile_options(-Wno-switch -Wno-format)
endif()

# Macro to create and install FMU libraries
macro(add_fmu_library LIB_NAME)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  file(GLOB ${LIB_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}/sources/*.c)
  add_library(${LIB_NAME} SHARED ${${LIB_NAME}_SRC})
  set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

  if(ARG_LIBRARIES)
    target_link_libraries(${LIB_NAME} PUBLIC ${ARG_LIBRARIES})
  endif()

  install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}/binaries/${ARCH_PAIR}
  )
  install(
    FILES ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}/modelDescription.xml
    DESTINATION ${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}
  )

  install(CODE "
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar -cf ${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}.fmu --format=zip .
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}
    )
    message(STATUS \"Created FMU package: ${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}.fmu\")
  ")
endmacro()


#   _____ __  __ _   _               _ _       
#  |  ___|  \/  | | | |  _   _ _ __ (_) |_ ___ 
#  | |_  | |\/| | | | | | | | | '_ \| | __/ __|
#  |  _| | |  | | |_| | | |_| | | | | | |_\__ \
#  |_|   |_|  |_|\___/   \__,_|_| |_|_|\__|___/
# 

# Create linear_axis FMU library
add_fmu_library(linear_axis)
