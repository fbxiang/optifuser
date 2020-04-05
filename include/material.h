#pragma once
#include <string>
#include <memory>
#include "texture.h"
#include <glm/glm.hpp>

namespace Optifuser {


struct PBRMaterial {
  std::string name = "";
  std::shared_ptr<Texture> kd_map = std::make_shared<Texture>();  // diffuse color
  std::shared_ptr<Texture> ks_map = std::make_shared<Texture>();  // specular ((ior-1)/(ior+1))^2/0.08
  std::shared_ptr<Texture> height_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> normal_map = std::make_shared<Texture>();

  glm::vec4 kd = glm::vec4(0, 0, 0, 1);
  float ks = 0.f;
  float roughness = 0.85f;
  float metallic = 0.f;
  bool forceTransparency = false;
};

}
