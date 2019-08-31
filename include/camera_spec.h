#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Optifuser {

class CameraSpec {
public:
  std::string name;
  glm::vec3 position;
  glm::quat rotation;
  float near = 0.1f;
  float far = 1000.f;
  float fovy = glm::radians(35.f);
  float aspect = 1.f;

  void lookAt(const glm::vec3 &direction, const glm::vec3 &up);

  inline glm::mat4 getModelMat() const {
    glm::mat4 t = glm::toMat4(rotation);
    t[3][0] = position.x;
    t[3][1] = position.y;
    t[3][2] = position.z;
    return t;
  }

  inline glm::mat4 getViewMat() const { return glm::inverse(getModelMat()); }

  virtual inline glm::mat4 getViewMatLocal() const { return getViewMat(); }

  inline glm::mat4 getProjectionMat() const {
    return glm::perspective(fovy, aspect, near, far);
  }
};

class FPSCameraSpec : public CameraSpec {
private:
  float yaw = 0.f;
  float pitch = 0.f;

public:
  glm::vec3 forward = {0, 0, -1};
  glm::vec3 up = {0, 1, 0};

  void lookAt(const glm::vec3 &direction);

  inline void setForward(const glm::vec3 &dir) { forward = dir; }

  inline void setUp(const glm::vec3 &dir) { up = dir; }

  bool isSane() const;

  void rotateYawPitch(float d_yaw, float d_pitch);

  void moveForwardRight(float d_forward, float d_right);

  glm::quat getRotation0() const;

  virtual glm::mat4 getViewMatLocal() const;
};

} // namespace Optifuser
