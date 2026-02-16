

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

set(FMI_PATH ${CMAKE_CURRENT_LIST_DIR}/FMI-Standard)

if(NOT EXISTS ${FMI_PATH})
  message(STATUS "Downloading FMI-Standard package...")
  file(DOWNLOAD https://github.com/modelica/fmi-standard/releases/download/v3.0.2/FMI-Standard-3.0.2.zip ${FMI_PATH}/FMI-Standard-3.0.2.zip)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${FMI_PATH}/FMI-Standard-3.0.2.zip WORKING_DIRECTORY ${FMI_PATH})
  message(STATUS "done.")
endif()

include_directories(${FMI_PATH}/headers)
add_compile_options(-Wno-switch -Wno-format)

file(GLOB LINEAR_AXIS_SRC ${CMAKE_CURRENT_LIST_DIR}/linear_axis/sources/*.c)
add_library(linear_axis SHARED ${LINEAR_AXIS_SRC})
set_target_properties(linear_axis PROPERTIES PREFIX "")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/fmu)
install(TARGETS linear_axis
  LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/fmu/linear_axis/${ARCH_PAIR}/binaries
)
install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/linear_axis/modelDescription.xml
  DESTINATION ${CMAKE_CURRENT_LIST_DIR}/fmu/linear_axis
)

install(CODE "
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -cf ${CMAKE_CURRENT_LIST_DIR}/fmu/linear_axis.fmu --format=zip .
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/fmu/linear_axis
  )
  message(STATUS \"Created FMU package: ${CMAKE_CURRENT_LIST_DIR}/linear_axis.fmu\")
")

