#
# Base CMake file
#
# Defines common settings, two further CMake files are used:
# 1. source/impl/CMakeLists.txt
# 2. source/bundle/CMakeLists.txt
#

cmake_minimum_required(VERSION 3.10)

set(CELIX_NO_POSTFIX_BUILD_TYPES RelWithDebInfo release debug Release Debug CACHE STRING "The build type used for creating bundle without a build type postfix.")


#
# Project information
#
project(sdk_example VERSION 3.0.0)

#
# The root directory of the ctrlX AUTOMATION SDK
#
set(SDK_ROOT_DIR "/home/boschrexroth/com.boschrexroth.sdk.control.rt-3.4.1/public")

#
# Set target dependend variables
#
if(${BUILD_KIT} MATCHES "aarch64")
  set(TARGET_PLATFORM "ubuntu22-gcc-aarch64")
  set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
  set(USR_LIB_DIR /usr/lib/aarch64-linux-gnu)
else()
  set(TARGET_PLATFORM "ubuntu22-gcc-x64")
  set(CMAKE_CXX_COMPILER g++)
  set(USR_LIB_DIR /usr/lib/x86_64-linux-gnu)
endif()

#
# Compiler setting
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-rtti -fno-gnu-unique")

#
# List important variables
#
message(STATUS "")
message(STATUS "Common settings ========================================================================")
message(STATUS "PROJECT_VERSION=" ${PROJECT_VERSION})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_STANDARD=" ${CMAKE_CXX_STANDARD})
message(STATUS "SDK_ROOT_DIR=" ${SDK_ROOT_DIR})
message(STATUS "TARGET_PLATFORM=" ${TARGET_PLATFORM})
message(STATUS "USR_LIB_DIR=" ${USR_LIB_DIR})
message(STATUS "========================================================================================")
message(STATUS "")

#
# Add these directories to the project
#
add_subdirectory(source/impl)
# Important: Add the bundle subdirectory last
add_subdirectory(source/bundle)
