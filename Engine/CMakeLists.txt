cmake_minimum_required(VERSION 3.22)
project(Engine)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
add_library(${PROJECT_NAME} ${SOURCES})

# Includes
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  glm
)
