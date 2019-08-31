#pragma once

#include "object.h"
#include <glm/glm.hpp>
namespace Optifuser {
class Camera : public Object {
public:
  float fovy;
  float aspect;
  float near = 0.1f;
  float far = 1000.f;
  bool zup = false;

 private : float yaw = 0;
  float pitch = 0;

public:
  Camera() : fovy(glm::radians(35.f)), aspect(1.f) { visible = false; }

  glm::mat4 getProjectionMat() const;
  glm::mat4 getViewMat() const;

  void move(float up, float right, float forward);
  void rotateYaw(float rad);
  void rotatePitch(float rad);

  float getYaw() const { return yaw; }
  float getPitch() const { return pitch; }
};

} // namespace Optifuser
