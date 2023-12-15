#
# Controls the build of a shared object library from the sources of this folder - see add_library(...)
#
# This library will be added to the bundle zip file together with the bundle so file.
# IMPORTANT: It must have a unique name: lib<unique name>.so
#

# Trunk name of the shared object library
set(IMPL_LIB sdk_example_lib)

add_library(${IMPL_LIB} SHARED
  trace.cpp
  rt_application.cpp
  rt_applicationFactory.cpp
)

target_include_directories(${IMPL_LIB}
  PRIVATE ${SDK_ROOT_DIR}/include/common.scheduler
  PRIVATE ${SDK_ROOT_DIR}/include/comm.datalayer
  PRIVATE ${SDK_ROOT_DIR}/include/common.log.trace
)

# don't set RPATH in shared library of activator because possibility wrong build paths in *.so
set_target_properties(${IMPL_LIB} PROPERTIES SKIP_BUILD_RPATH true)

# Neccessary for RtTrace
target_link_libraries(${IMPL_LIB}
  ${SDK_ROOT_DIR}/lib/common.log.trace/${TARGET_PLATFORM}/release/libcommon_log_trace_buffered_static.a
)

# single-configuration generator (Unix Makefile)
install(TARGETS ${IMPL_LIB}
  RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/generated/${TARGET_PLATFORM}/${CMAKE_BUILD_TYPE}
  LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/generated/${TARGET_PLATFORM}/${CMAKE_BUILD_TYPE}
  ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/generated/${TARGET_PLATFORM}/${CMAKE_BUILD_TYPE}
)
