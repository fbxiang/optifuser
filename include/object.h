#pragma once
#include "shader.h"
#include <memory>
#include <string>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "mesh.h"
#include "noise/perlin.h"
#include "texture.h"
namespace Optifuser {
class Scene;

struct Material {
  std::string name = "";
  std::string type = "regular";

  std::shared_ptr<Texture> kd_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> ks_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> height_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> normal_map = std::make_shared<Texture>();

  glm::vec3 kd = glm::vec3(0);
  glm::vec3 ks = glm::vec3(1);
  glm::vec3 ka = glm::vec3(0);
  float exp = 1.f;
};

class Object : public std::enable_shared_from_this<Object> {
protected:
  std::shared_ptr<AbstractMeshBase> mesh;

public:
  std::shared_ptr<Shader> shader;
  Material material;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  bool visible;

protected:
  std::weak_ptr<Scene> scene;

public:
  Object(std::shared_ptr<AbstractMeshBase> m = nullptr)
      : mesh(m), shader(nullptr), name(""), position(0.f), scale(1.f),
        rotation(), visible(true) {}

  virtual ~Object() {}

  glm::mat4 getModelMat() const;
  void setScene(const std::shared_ptr<Scene> inScene);
  std::shared_ptr<Scene> getScene() const;

  std::shared_ptr<AbstractMeshBase> getMesh() const;
};

template <typename T>
std::shared_ptr<T> NewObject(std::shared_ptr<AbstractMeshBase> mesh) {
  static_assert(std::is_base_of<Object, T>::value,
                "T must inherit from Obejct.");
  auto obj = std::make_shared<T>(mesh);
  return obj;
}

template <typename T> std::shared_ptr<T> NewObject() {
  static_assert(std::is_base_of<Object, T>::value,
                "T must inherit from Obejct.");
  auto obj = std::make_shared<T>();
  return obj;
}

std::shared_ptr<Object> NewNoisePlane(unsigned int res);
std::shared_ptr<Object> NewDebugObject();
std::shared_ptr<Object> NewPlane();
std::shared_ptr<Object> NewYZPlane();
std::shared_ptr<Object> NewCube();
std::shared_ptr<Object> NewSphere();
std::shared_ptr<Object> NewLine();
std::shared_ptr<Object> NewLineCube();
std::shared_ptr<Object> NewEnvironmentCube();

} // namespace Optifuser
