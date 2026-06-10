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
    set(ARCH_PAIR "x86_64-windows")
  else()
    set(ARCH_PAIR "aarch64-windows")
  endif()
else()
  message(WARNING "Unknown platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
  set(ARCH_PAIR "unknown")
endif()
message(STATUS "FMI3 Architecture pair: ${ARCH_PAIR}")
set(FMU_INSTALL_COMPONENT fmu)
set(FMU_TARGETS)

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

if (UNIX)
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

  set(FMU_DESTINATION "${CMAKE_CURRENT_LIST_DIR}/fmu/${LIB_NAME}")
  message(STATUS "FMU destination: ${FMU_DESTINATION}")
  list(APPEND FMU_TARGETS ${LIB_NAME})
  install(TARGETS ${LIB_NAME}
    LIBRARY
      DESTINATION "${FMU_DESTINATION}/binaries/${ARCH_PAIR}"
      COMPONENT ${FMU_INSTALL_COMPONENT}
      EXCLUDE_FROM_ALL
    RUNTIME
      DESTINATION "${FMU_DESTINATION}/binaries/${ARCH_PAIR}"
      COMPONENT ${FMU_INSTALL_COMPONENT}
      EXCLUDE_FROM_ALL
  )
  install(
    FILES ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}/modelDescription.xml
    DESTINATION ${FMU_DESTINATION}
    COMPONENT ${FMU_INSTALL_COMPONENT}
    EXCLUDE_FROM_ALL
  )

  install(CODE "
    file(REMOVE \"${FMU_DESTINATION}.fmu\")

    if(WIN32)
      execute_process(
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command \"Compress-Archive -Path '${FMU_DESTINATION}/*' -DestinationPath '${FMU_DESTINATION}.fmu' -Force\"
        RESULT_VARIABLE fmu_archive_result
        ERROR_VARIABLE fmu_archive_error
      )
    else()
      execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E tar cf \"${FMU_DESTINATION}.fmu\" --format=zip .
        WORKING_DIRECTORY \"${FMU_DESTINATION}\"
        RESULT_VARIABLE fmu_archive_result
        ERROR_VARIABLE fmu_archive_error
      )
    endif()

    if(NOT fmu_archive_result EQUAL 0)
      message(FATAL_ERROR \"Failed to create FMU package ${FMU_DESTINATION}.fmu: \${fmu_archive_error}\")
    endif()

    if(NOT EXISTS \"${FMU_DESTINATION}.fmu\")
      message(FATAL_ERROR \"FMU package was not created: ${FMU_DESTINATION}.fmu\")
    endif()

    message(STATUS \"Created FMU package: ${FMU_DESTINATION}.fmu\")
  "
    COMPONENT ${FMU_INSTALL_COMPONENT}
    EXCLUDE_FROM_ALL
  )
endmacro()


#   _____ __  __ _   _               _ _       
#  |  ___|  \/  | | | |  _   _ _ __ (_) |_ ___ 
#  | |_  | |\/| | | | | | | | | '_ \| | __/ __|
#  |  _| | |  | | |_| | | |_| | | | | | |_\__ \
#  |_|   |_|  |_|\___/   \__,_|_| |_|_|\__|___/
# 

# Create linear_axis FMU library
add_fmu_library(linear_axis)
add_fmu_library(DoubleMassSpringDamper)
add_fmu_library(machine_tool)

add_custom_target(install_fmu
  COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}"
          --component ${FMU_INSTALL_COMPONENT}
          --config "$<CONFIG>"
  DEPENDS ${FMU_TARGETS}
  USES_TERMINAL
  COMMENT "Installing FMUs into ${CMAKE_CURRENT_LIST_DIR}/fmu"
)
