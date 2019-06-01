#pragma once
#include <glm/glm.hpp>
namespace Optifuser {
struct PointLight {
  glm::vec3 position;
  glm::vec3 emission;
};

struct DirectionalLight {
  glm::vec3 direction;
  glm::vec3 emission;
};

struct ParallelogramLight {
  glm::vec3 corner;
  glm::vec3 v1, v2;
  glm::vec3 normal;
  glm::vec3 emission;
};

} // namespace Optifuser
