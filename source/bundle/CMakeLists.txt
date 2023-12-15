

#
# CMake project file to ...
# - compile the sources under private/bundle
# - create the Celix activator/component so file (lib...so)
# - create the Celix bundle file (...zip)
# - if debug build: search all bundles (zip file) and create a config.properties file to start the Celix framework within this build environmant
#

# The name of the so file created: lib....
set(ACTIVATOR_LIB ExampleBundle)

set(BUNDLEX_NAME         "Example.Bundle")
set(BUNDLEX_DISPLAY_NAME "ExampleBundle")
set(BUNDLEX_VERSION ${PROJECT_VERSION}) # Set by root CMakeLists.txt, can be modified if more than one bundle is deployed
set(BUNDLEX_DESCRIPTION "Bundle example to demonstrate scheduler usage.")

# CELIX_DEPENDENCY_DIR is used by CelixConfig.cmake
set(CELIX_DEPENDENCY_DIR ${SDK_ROOT_DIR})
set(CELIX_LIBRARIES ${SDK_ROOT_DIR}/bin/oss.celix/${TARGET_PLATFORM}/release)
set(CELIX_DM_STATIC_CXX_LIB ${CELIX_LIBRARIES}/libcelix_dependency_manager_cxx_static.a)
set(CELIX_CMAKECELIX_FILE ${SDK_ROOT_DIR}/cmake/oss.celix/CMakeCelix.cmake)

# Add Celix CMake folder to CMAKE_PREFIX_PATH used by find_package()
set(CMAKE_PREFIX_PATH ${SDK_ROOT_DIR}/cmake/oss.celix ${CMAKE_PREFIX_PATH})

# Find and load settings of required package Celix
find_package(Celix REQUIRED)

message(STATUS "")
message(STATUS "Celix bundles setting ==================================================================")
message(STATUS "ACTIVATOR_LIB=" ${ACTIVATOR_LIB})
message(STATUS "BUNDLEX_DESCRIPTION=" ${BUNDLEX_DESCRIPTION})
message(STATUS "BUNDLEX_DISPLAY_NAME=" ${BUNDLEX_DISPLAY_NAME})
message(STATUS "BUNDLEX_NAME=" ${BUNDLEX_NAME})
message(STATUS "BUNDLEX_VERSION=" ${BUNDLEX_VERSION})
message(STATUS "CELIX_CMAKECELIX_FILE=" ${CELIX_CMAKECELIX_FILE})
message(STATUS "CELIX_DEPENDENCY_DIR=" ${CELIX_DEPENDENCY_DIR})
message(STATUS "CELIX_DM_STATIC_CXX_LIB=" ${CELIX_DM_STATIC_CXX_LIB})
message(STATUS "CELIX_LIBRARIES=" ${CELIX_LIBRARIES})
message(STATUS "CMAKE_PREFIX_PATH=" ${CMAKE_PREFIX_PATH})
message(STATUS "========================================================================================")
message(STATUS "")

# Use these source files for our library
add_library(${ACTIVATOR_LIB} SHARED
  activator.cpp
  cmp.cpp
)

# Do not add Runtime Path (RPATH) to the header of the so file
set_target_properties(${ACTIVATOR_LIB} PROPERTIES SKIP_BUILD_RPATH true)

add_dependencies(${ACTIVATOR_LIB} sdk_example_lib)
# Use these include directories
target_include_directories(${ACTIVATOR_LIB}
   PRIVATE ${SDK_ROOT_DIR}/include/oss.celix/celix
   PRIVATE ${SDK_ROOT_DIR}/include/common.scheduler
   PRIVATE ${SDK_ROOT_DIR}/include/comm.datalayer
   PRIVATE ${SDK_ROOT_DIR}/include/common.log.trace
   PRIVATE ../impl
)
#
# Call the Celix CMake function to build a bundle
#
target_link_libraries(${ACTIVATOR_LIB}
                        sdk_example_lib)
add_celix_bundle(${BUNDLEX_NAME}
  SYMBOLIC_NAME ${BUNDLEX_DISPLAY_NAME}
  VERSION ${BUNDLEX_VERSION}
  ACTIVATOR ${ACTIVATOR_LIB}
  DESCRIPTION ${BUNDLEX_DESCRIPTION}
  EXPORT_LIBRARIES
    sdk_example_lib
)
#
# Copy bundle zip file
#
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLEX_NAME}.zip DESTINATION ${CMAKE_SOURCE_DIR}/generated/bundle)
#
# Only for debugging in App Build Environment:
# - Create generated/config.properties for Celix (rexroth-automation-frame)
# - Search and add ctrlX AUTOMATION bundles
# - Search and add Celix bundles
# - Add our own bundle
# - Add some switches (options)
#
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  message(STATUS "Creating generated/config.properties, searching for bundles to be started ...")

  set(BUNDLE_LIST "")
  file(GLOB_RECURSE BUNDLES_CXA ${SDK_ROOT_DIR}/bundle/*/ubuntu22-gcc-x64/*.zip)

  foreach(BUNDLE ${BUNDLES_CXA})
    string(REPLACE ${CMAKE_SOURCE_DIR}/ "" BUNDLE_RELATIVE ${BUNDLE})
    message(STATUS "Found ctrlX AUTOMATION bundle ${BUNDLE_RELATIVE}")
    list(APPEND BUNDLE_LIST ${BUNDLE_RELATIVE})
  endforeach()

  file(GLOB_RECURSE BUNDLES_CELIX ${SDK_ROOT_DIR}/bin/framework/ubuntu22-gcc-x64/bundle/*.zip)

  foreach(BUNDLE ${BUNDLES_CELIX})
    string(REPLACE ${CMAKE_SOURCE_DIR}/ "" BUNDLE_RELATIVE ${BUNDLE})
    message(STATUS "Found Celix bundle ${BUNDLE_RELATIVE}")
    list(APPEND BUNDLE_LIST ${BUNDLE_RELATIVE})
  endforeach()

  set(BUNDLE_RELATIVE generated/bundle/${BUNDLEX_NAME}.zip)
  message(STATUS "Adding our bundle ${BUNDLE_RELATIVE}")
  list(APPEND BUNDLE_LIST ${BUNDLE_RELATIVE})

  string(REPLACE ";" " " START_LIST "${BUNDLE_LIST}")
  file(GENERATE
    OUTPUT "${CMAKE_SOURCE_DIR}/generated/config.properties"
    CONTENT "cosgi.auto.start.1=${START_LIST}\nlicense.proxy.debug=true # Disables the license check\ntokencheck.enable=false # Disables token check of datalayer TCP connections")
endif()




