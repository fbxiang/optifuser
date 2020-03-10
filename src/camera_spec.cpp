#include <camera_spec.h>

namespace Optifuser {

bool is_close(float f1, float f2, float eps = 1e-4) { return f1 - f2 > -eps && f1 - f2 < eps; }

bool FPSCameraSpec::isSane() const {
  return is_close(glm::length(forward), 1.f) && is_close(glm::length(up), 1.f) &&
         is_close(glm::dot(forward, up), 0.f);
}

void FPSCameraSpec::rotateYawPitch(float d_yaw, float d_pitch) {
  yaw += d_yaw;
  pitch += d_pitch;
  if (yaw >= glm::pi<float>()) {
    yaw -= 2 * glm::pi<float>();
  } else if (yaw < -glm::pi<float>()) {
    yaw += 2 * glm::pi<float>();
  }
  pitch = glm::clamp(pitch, -glm::pi<float>() / 2 + 0.05f, glm::pi<float>() / 2 - 0.05f);
  glm::vec3 right = glm::cross(forward, up);
  rotation = glm::angleAxis(yaw, up) * glm::angleAxis(pitch, right) * getRotation0();
}

void FPSCameraSpec::moveForwardRight(float d_forward, float d_right) {
  glm::mat4 model = getModelMat();

  glm::vec3 forward_global = model * glm::vec4(0, 0, -1, 0);
  glm::vec3 right_global = glm::cross(forward_global, up);

  position += d_forward * forward_global + d_right * right_global;
}

glm::quat FPSCameraSpec::getRotation0() const {
  glm::mat3 mat = glm::mat3(glm::cross(forward, up), up, -forward);
  return glm::quat(mat);
}

void FPSCameraSpec::update() {
  glm::vec3 right = glm::cross(forward, up);
  rotation = glm::angleAxis(yaw, up) * glm::angleAxis(pitch, right) * getRotation0();
}

} // namespace Optifuser
