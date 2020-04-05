#include "scene.h"
#include "texture.h"
#include <algorithm>
namespace Optifuser {

void Scene::addObject(std::unique_ptr<Object> obj) {
  obj->setScene(this);
  objects.push_back(std::move(obj));
}

void Scene::removeObject(Object *obj) {
  auto s = obj->getScene();
  if (s != this) {
    return;
  }
  obj->setScene(nullptr);
  objects.erase(std::remove_if(objects.begin(), objects.end(),
                               [obj](std::unique_ptr<Object> &o) { return obj == o.get(); }),
                objects.end());
}

void Scene::removeObjectsByName(std::string name) {
  objects.erase(std::remove_if(objects.begin(), objects.end(),
                               [&](std::unique_ptr<Object> &o) { return o->name == name; }),
                objects.end());
}

void Scene::setAmbientLight(glm::vec3 light) { ambientLight = light; }
void Scene::addPointLight(PointLight light) { pointLights.push_back(light); }
void Scene::addDirectionalLight(DirectionalLight light) { directionalLights.push_back(light); }
void Scene::setShadowLight(DirectionalLight light) {
  if (!directionalLights.size()) {
    addDirectionalLight(light);
  } else {
    directionalLights[0] = light;
  }
}
void Scene::addParalleloGramLight(ParallelogramLight light) {
  parallelogramLights.push_back(light);
}

void Scene::setEnvironmentMap(const std::string &front, const std::string &back,
                              const std::string &top, const std::string &bottom,
                              const std::string &left, const std::string &right, int wrapping,
                              int filtering) {
  environmentMap = LoadCubeMapTexture(front, back, top, bottom, left, right, wrapping, filtering);
}

static void prepareObjectTree(Object *obj, const glm::mat4 &parentModelMat,
                              std::vector<Object *> &opaque, std::vector<Object *> &transparent) {
  obj->globalModelMatrix = parentModelMat * obj->getModelMat();
  if (obj->getMesh() && obj->visible) {
    if (obj->pbrMaterial->forceTransparency ||
        (!obj->pbrMaterial->kd_map->getId() && obj->pbrMaterial->kd.a < 1)) {
      transparent.push_back(obj);
    } else {
      opaque.push_back(obj);
    }
  }
  for (auto &c : obj->getChildren()) {
    prepareObjectTree(c.get(), obj->globalModelMatrix, opaque, transparent);
  }
}

void Scene::prepareObjects() {
  opaque_objects.clear();
  transparent_objects.clear();
  for (auto &obj : objects) {
    prepareObjectTree(obj.get(), glm::mat4(1.f), opaque_objects, transparent_objects);
  }
}

} // namespace Optifuser
