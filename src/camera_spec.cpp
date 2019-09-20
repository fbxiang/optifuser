#include <camera_spec.h>

namespace Optifuser {

bool is_close(float f1, float f2, float eps = 1e-4) {
  return f1 - f2 > -eps && f1 - f2 < eps;
}

void CameraSpec::lookAt(const glm::vec3 &direction, const glm::vec3 &up) {
  rotation = glm::quatLookAt(glm::normalize(direction), up);
}

void FPSCameraSpec::lookAt(const glm::vec3 &direction) {
  CameraSpec::lookAt(direction, up);
}

bool FPSCameraSpec::isSane() const {
  return is_close(glm::length(forward), 1.f) &&
         is_close(glm::length(up), 1.f) && is_close(glm::dot(forward, up), 0.f);
}

void FPSCameraSpec::rotateYawPitch(float d_yaw, float d_pitch) {
  yaw += d_yaw;
  pitch += d_pitch;
  if (yaw >= M_PIf32) {
    yaw -= 2 * M_PIf32;
  } else if (yaw < -M_PIf32) {
    yaw += 2 * M_PIf32;
  }
  pitch = glm::clamp(pitch, -M_PIf32 / 2 + 0.05f, M_PIf32 / 2 - 0.05f);
  glm::vec3 right = glm::cross(forward, up);
  rotation =
      glm::angleAxis(yaw, up) * glm::angleAxis(pitch, right) * getRotation0();
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

glm::mat4 FPSCameraSpec::getViewMatLocal() const {
  glm::quat rotation_local = glm::angleAxis(yaw, glm::vec3(0, 1, 0)) *
                             glm::angleAxis(pitch, glm::vec3(1, 0, 0));
  glm::mat4 t = glm::toMat4(rotation_local);
  t[3][0] = position.x;
  t[3][1] = position.y;
  t[3][2] = position.z;
  return glm::inverse(t);
}

void FPSCameraSpec::update() {
  glm::vec3 right = glm::cross(forward, up);
  rotation = glm::angleAxis(yaw, up) * glm::angleAxis(pitch, right) * getRotation0();
}

} // namespace Optifuser
