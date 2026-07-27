cmake_minimum_required(VERSION 3.22)
project(DirectX12)

# Parameters
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
  -DUNICODE
  -D_UNICODE
  -DNOMINMAX
  -DGLM_ENABLE_EXPERIMENTAL
  -DGLM_FORCE_RADIANS
  -DGLM_FORCE_LEFT_HANDED
  -DGLM_FORCE_DEPTH_ZERO_TO_ONE
)

if (MSVC)
  add_compile_options(/utf-8)
endif ()

# External dependencies
add_subdirectory(vendor)

# Project modules
add_subdirectory(Engine)
add_subdirectory(Project)
