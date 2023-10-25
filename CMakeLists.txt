cmake_minimum_required(VERSION 3.18)

option(METAPP_PEDANTIC "Pedantic flags." OFF)

project(metapp)

add_library(${PROJECT_NAME} INTERFACE)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
)

target_include_directories(${PROJECT_NAME}
  INTERFACE "include/"
)

if (METAPP_PEDANTIC)
  target_compile_options(${PROJECT_NAME}
    INTERFACE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>: -Wall -Wextra -Wpedantic -Werror -Wsign-conversion -Wconversion>
      $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>
  )
endif()
