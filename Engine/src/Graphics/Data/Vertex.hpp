#pragma once
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Flame {
  struct Vertex final {
    glm::vec3 position;
    glm::vec4 color;
  };
}
