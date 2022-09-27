cmake_minimum_required(VERSION 3.18)
project(dune-localfefunctions CXX)



if(NOT (dune-common_DIR OR dune-common_ROOT OR
      "${CMAKE_PREFIX_PATH}" MATCHES ".*dune-common.*"))
    string(REPLACE  ${PROJECT_NAME} dune-common dune-common_DIR
      ${PROJECT_BINARY_DIR})
endif()

#find dune-common and set the module path
find_package(dune-common REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules"
  ${dune-common_MODULE_PATH})

#include the dune macros
include(DuneMacros)

# start a dune project with information from dune.module
dune_project()
dune_add_library(dune-localfefunctions)
target_compile_features(dune-localfefunctions PRIVATE cxx_std_20)
find_package(Eigen3 3.3.9 REQUIRED)
dune_enable_all_packages()



add_subdirectory(dune)
add_subdirectory(doc)
add_subdirectory(cmake/modules)

# finalize the dune project, e.g. generating config.h etc.
finalize_dune_project(GENERATE_CONFIG_H_CMAKE)