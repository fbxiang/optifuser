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
  enum Type { SOLID, TRANSPARENT, MIRROR } type = SOLID;

  std::string name = "";

  std::shared_ptr<Texture> kd_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> ks_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> height_map = std::make_shared<Texture>();
  std::shared_ptr<Texture> normal_map = std::make_shared<Texture>();

  glm::vec4 kd = glm::vec4(0, 0, 0, 1);
  glm::vec3 ks = glm::vec3(0);
  glm::vec3 ka = glm::vec3(0);
  float exp = 1.f;
};

class Object {
protected:
  std::shared_ptr<AbstractMeshBase> mesh;
  Object *parent;
  std::vector<std::unique_ptr<Object>> children;

  uint32_t segmentId = 0; // used for rendering segmentation
  uint32_t objId = 0;     // used for rendering fine segmentation

  std::vector<float> userData = {0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0};

public:
  std::shared_ptr<Shader> shader;
  Material material;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  bool visible;

  uint32_t showAxis = 0; // used for showing axis

protected:
  glm::quat rotation;
  Scene *scene;

public:
  Object(std::shared_ptr<AbstractMeshBase> m = nullptr)
      : mesh(m), parent(nullptr), shader(nullptr), name(""), position(0.f),
        scale(1.f), visible(true), rotation(1, 0, 0, 0), scene(nullptr) {}

  virtual ~Object() {}

  inline void setRotation(glm::quat const &rot) {
    rotation = glm::normalize(rot);
  }

  inline glm::quat const &getRotation() const { return rotation; }

  glm::mat4 getModelMat() const;
  void setScene(Scene *inScene);
  Scene *getScene() const;

  std::shared_ptr<AbstractMeshBase> getMesh() const;

  void addChild(std::unique_ptr<Object> child);
  inline const std::vector<std::unique_ptr<Object>> &getChildren() const {
    return children;
  }

  inline void setObjId(uint32_t id) { objId = id; }
  inline uint32_t getObjId() const { return objId; }
  inline void setSegmentId(uint32_t id) { segmentId = id; }
  inline uint32_t getSegmentId() const { return segmentId; }
  inline void setUserData(std::vector<float> const &data) { userData = data; }
  inline std::vector<float> const &getUserData() const { return userData; }
};

template <typename T>
std::unique_ptr<T> NewObject(std::shared_ptr<AbstractMeshBase> mesh) {
  static_assert(std::is_base_of<Object, T>::value,
                "T must inherit from Obejct.");
  auto obj = std::make_unique<T>(mesh);
  return obj;
}

template <typename T> std::unique_ptr<T> NewObject() {
  static_assert(std::is_base_of<Object, T>::value,
                "T must inherit from Obejct.");
  auto obj = std::make_unique<T>();
  return obj;
}

std::unique_ptr<Object> NewNoisePlane(unsigned int res);
std::unique_ptr<Object> NewDebugObject();
std::unique_ptr<Object> NewXYPlane();
std::unique_ptr<Object> NewYZPlane();
std::unique_ptr<Object> NewFlatCube();
std::unique_ptr<Object> NewCube();
std::unique_ptr<Object> NewSphere();
std::unique_ptr<Object> NewLine();
std::unique_ptr<Object> NewLineCube();
std::unique_ptr<Object> NewMeshGrid();
std::unique_ptr<Object> NewAxes();
std::unique_ptr<Object> NewCapsule(float halfHeight, float radius);

} // namespace Optifuser
