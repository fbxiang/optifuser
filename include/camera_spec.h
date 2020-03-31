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
  glm::vec3 position = {0, 0, 0};
  float near = 0.1f;
  float far = 1000.f;
  float aspect = 1.f;

protected:
  glm::quat rotation = {1, 0, 0, 0};

public:
  inline void lookAt(const glm::vec3 &direction, const glm::vec3 &up) {
    rotation = glm::quatLookAt(glm::normalize(direction), up);
  }
  inline void setRotation(glm::quat const &rot) { rotation = glm::normalize(rot); }
  inline glm::quat const &getRotation() const { return rotation; }

  inline glm::mat4 getModelMat() const {
    glm::mat4 t = glm::toMat4(rotation);
    t[3][0] = position.x;
    t[3][1] = position.y;
    t[3][2] = position.z;
    return t;
  }

  inline glm::mat4 getViewMat() const { return glm::inverse(getModelMat()); }

  virtual ~CameraSpec() = default;

  virtual glm::mat4 getProjectionMat() const = 0;
  virtual float getFovy() const = 0;
};

class OrthographicCameraSpec : public CameraSpec {
public:
  float scaling = 1.f;

public:
  inline glm::mat4 getProjectionMat() const override {
    return glm::ortho(-scaling * aspect, scaling * aspect, -scaling, scaling, near, far);
  }

  float getFovy() const override { return 0; }
};

class PerspectiveCameraSpec : public CameraSpec {
public:
  float fovy = glm::radians(35.f);

public:
  inline glm::mat4 getProjectionMat() const override {
    return glm::perspective(fovy, aspect, near, far);
  }

  float getFovy() const override { return fovy; }
};

class FPSCameraSpec : public PerspectiveCameraSpec {
private:
  float yaw = 0.f;
  float pitch = 0.f;
  glm::vec3 forward = {0, 0, -1};
  glm::vec3 up = {0, 1, 0};

public:
  // updates rotation based on forward and up
  void update();

  inline void setForward(const glm::vec3 &dir) { forward = dir; }

  inline void setUp(const glm::vec3 &dir) { up = dir; }

  bool isSane() const;

  void rotateYawPitch(float d_yaw, float d_pitch);

  void moveForwardRight(float d_forward, float d_right);

  glm::quat getRotation0() const;
};

} // namespace Optifuser
