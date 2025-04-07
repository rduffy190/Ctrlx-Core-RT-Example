#
# CMake project file to ...
# - compile the sources under src
# - create the Celix activator/component so file (lib...so)
# - create the Celix bundle file (...zip)
# - if debug build: search all bundles (zip file) and create a config.properties file to start the Celix framework within this build environmant
#

# The name of the shared object file created: lib....so - it must be unique among all installed bundles!
set(ACTIVATOR_LIB sdk_rt_di_do)

set(BUNDLEX_NAME         "com.boschrexroth.sdk.rt.template")
set(BUNDLEX_DISPLAY_NAME "com_boschrexroth_sdk_rt_template")
set(BUNDLEX_VERSION ${PROJECT_VERSION})
set(BUNDLEX_DESCRIPTION "Bundle example to demonstrate scheduler usage.")


# CELIX_DEPENDENCY_DIR is used by CelixConfig.cmake
set(CELIX_DEPENDENCY_DIR ${SDK_ROOT_DIR})
set(CELIX_LIBRARIES ${SDK_ROOT_DIR}/bin/oss.celix/${TARGET_PLATFORM})
set(CELIX_CMAKECELIX_FILE ${SDK_ROOT_DIR}/cmake/oss.celix/CMakeCelix.cmake)

# Add Celix CMake folder to CMAKE_PREFIX_PATH used by find_package()
set(CMAKE_PREFIX_PATH ${SDK_ROOT_DIR}/cmake/oss.celix ${CMAKE_PREFIX_PATH})

# Find and load settings of required package Celix
find_package(Celix REQUIRED)

# Use these source files for our library
add_library(${ACTIVATOR_LIB} SHARED
  BundleActivator.cpp
  trace/trace.cpp
  realtime_template/realtime_template.cpp
  realtime_template/realtime_template_callable.cpp
  realtime_template/realtime_template_callable_factory.cpp
)

# Do not add Runtime Path (RPATH) to the header of the so file
set_target_properties(${ACTIVATOR_LIB} PROPERTIES SKIP_BUILD_RPATH true)

# Use these include directories
target_include_directories(${ACTIVATOR_LIB}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${SDK_ROOT_DIR}/include/oss.celix
  PRIVATE ${SDK_ROOT_DIR}/include/comm.datalayer 
  PRIVATE ${SDK_ROOT_DIR}/include/common.log.trace 
  PRIVATE ${SDK_ROOT_DIR}/include/common.scheduler
  PRIVATE ${SDK_ROOT_DIR}/samples-cpp
)

#
# Add these libraries also to the target
# -Wl,xxx: -Wl, is used to forward the argument xxx (through cmake and g++) to the linker
#
target_link_libraries(${ACTIVATOR_LIB}
  -Wl,--no-whole-archive ${CELIX_LIBRARIES}
  ${SDK_ROOT_DIR}/lib/common.log.trace/${TARGET_PLATFORM}/libcommon_log_trace_buffered_static.a
  systemd # required by libcommon_log_trace_buffered_static.a
)

#
# Call the Celix CMake function to build a bundle
#
add_celix_bundle(${BUNDLEX_NAME}
  SYMBOLIC_NAME ${BUNDLEX_DISPLAY_NAME}
  VERSION ${BUNDLEX_VERSION}
  ACTIVATOR ${ACTIVATOR_LIB}
  DESCRIPTION ${BUNDLEX_DESCRIPTION}
)

#
# Copy bundle zip file
#
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLEX_NAME}.zip DESTINATION ${CMAKE_SOURCE_DIR}/generated/bundle)
