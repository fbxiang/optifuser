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
  std::remove_if(objects.begin(), objects.end(),
                 [obj](std::unique_ptr<Object> &o) { return obj == o.get(); }),
      objects.end();
}

void Scene::removeObjectsByName(std::string name) {
  std::remove_if(objects.begin(), objects.end(),
                 [&](std::unique_ptr<Object> &o) { return o->name == name; }),
      objects.end();
}

const std::vector<std::unique_ptr<Object>> &Scene::getObjects() const { return objects; }

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
} // namespace Optifuser
