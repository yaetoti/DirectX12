cmake_minimum_required(VERSION 3.22)
project(Project)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
  Engine
  d3d12.lib
  d3dcompiler.lib
  dxgi.lib
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)
