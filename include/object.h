#pragma once
#include "material.h"
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

class Object {
protected:
  std::shared_ptr<AbstractMeshBase> mesh;
  Object *parent = nullptr;
  std::vector<std::unique_ptr<Object>> children;

  uint32_t segmentId = 0; // used for rendering segmentation
  uint32_t objId = 0;     // used for rendering fine segmentation

  std::vector<float> userData = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

public:
  std::shared_ptr<Shader> shader = nullptr;
  std::shared_ptr<PBRMaterial> pbrMaterial = std::make_shared<PBRMaterial>();
  std::string name;
  glm::vec3 position = {0.f, 0.f, 0.f};
  glm::vec3 scale = {1.f, 1.f, 1.f};
  float visibility = 1.f;
  uint32_t showAxis = 0; // used for showing axis

  glm::mat4 globalModelMatrix; // cached at render time

protected:
  glm::quat rotation = glm::quat(1, 0, 0, 0);
  Scene *scene = nullptr;
  bool toRemove = false;

public:
  Object(std::shared_ptr<AbstractMeshBase> m = nullptr) : mesh(m) {}

  virtual ~Object() {}

  inline void setRotation(glm::quat const &rot) { rotation = glm::normalize(rot); }

  inline glm::quat const &getRotation() const { return rotation; }

  glm::mat4 getModelMat() const;
  void setScene(Scene *inScene);
  Scene *getScene() const;

  std::shared_ptr<AbstractMeshBase> getMesh() const;

  void addChild(std::unique_ptr<Object> child);
  inline const std::vector<std::unique_ptr<Object>> &getChildren() const { return children; }

  inline void setObjId(uint32_t id) { objId = id; }
  inline uint32_t getObjId() const { return objId; }
  inline void setSegmentId(uint32_t id) { segmentId = id; }
  inline uint32_t getSegmentId() const { return segmentId; }
  inline void setUserData(std::vector<float> const &data) { userData = data; }
  inline std::vector<float> const &getUserData() const { return userData; }
  inline void markRemoved() { toRemove = true; }
  inline bool isMarkedRemoved() { return toRemove; }
};

template <typename T> std::unique_ptr<T> NewObject(std::shared_ptr<AbstractMeshBase> mesh) {
  static_assert(std::is_base_of<Object, T>::value, "T must inherit from Obejct.");
  auto obj = std::make_unique<T>(mesh);
  return obj;
}

template <typename T> std::unique_ptr<T> NewObject() {
  static_assert(std::is_base_of<Object, T>::value, "T must inherit from Obejct.");
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

std::shared_ptr<PBRMaterial> createDefaultMaterial(std::string const &name);

} // namespace Optifuser
