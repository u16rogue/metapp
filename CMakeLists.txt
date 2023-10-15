cmake_minimum_required(VERSION 3.18)

project(metapp)

add_library(
  ${PROJECT_NAME}
  INTERFACE
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  CXX_STANDARD 20
)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE
  "include/"
)
