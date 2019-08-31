#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
namespace Optifuser {

glm::mat4 Camera::getProjectionMat() const {
  return glm::perspective(fovy, aspect, near, far);
}

glm::mat4 Camera::getViewMat() const {
  return glm::transpose(glm::toMat4(rotation)) *
         glm::translate(glm::mat4(1.0f), -position);
  // return glm::inverse(getModelMat());
}

void Camera::move(float up, float right, float forward) {
  glm::mat4 model = getModelMat();

  glm::vec3 r = glm::vec3(model * glm::vec4(1, 0, 0, 0));
  if (zup) {
    r[2] = 0;
  } else {
    r[1] = 0;
  }

  r = glm::normalize(r);

  glm::vec3 f = glm::vec3(model * glm::vec4(0, 0, -1, 0));
  glm::vec3 u = glm::vec3(0, 1, 0);

  position += right * r + up * u + forward * f;
}

void Camera::rotateYaw(float rad) {
  yaw += rad;
  rotation = glm::angleAxis(yaw, glm::vec3(0, 1, 0)) *
             glm::angleAxis(pitch, glm::vec3(1, 0, 0));
  auto r0 = glm::angleAxis(-M_PIf32 / 2, glm::vec3(0, 0, 1)) *
            glm::angleAxis(M_PIf32 / 2, glm::vec3(1, 0, 0));
  if (zup) {
    rotation = r0 * rotation;
  }
}

void Camera::rotatePitch(float rad) {
  pitch = glm::clamp(pitch + rad, -glm::pi<float>() / 2, glm::pi<float>() / 2);

  rotation = glm::angleAxis(yaw, glm::vec3(0, 1, 0)) *
             glm::angleAxis(pitch, glm::vec3(1, 0, 0));
  auto r0 = glm::angleAxis(-M_PIf32 / 2, glm::vec3(0, 0, 1)) *
            glm::angleAxis(M_PIf32 / 2, glm::vec3(1, 0, 0));
  if (zup) {
    rotation = r0 * rotation;
  }
}
} // namespace Optifuser
