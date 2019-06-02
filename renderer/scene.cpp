#include "scene.h"
#include "texture.h"
#include <algorithm>
namespace Optifuser {
bool Scene::contains(const std::shared_ptr<Object> obj) const {
  return obj->getScene() == shared_from_this();
}

void Scene::addObject(std::shared_ptr<Object> obj) {
  auto s = obj->getScene();
  if (s == shared_from_this())
    return;
  if (s) {
    s->removeObject(obj);
  }
  obj->setScene(shared_from_this());
  objects.push_back(obj);
}

// TODO: implement this function
void Scene::removeObject(std::shared_ptr<Object> obj) {
  auto s = obj->getScene();
  if (s != shared_from_this()) {
    return;
  }
  obj->setScene(nullptr);
  objects.erase(
      std::remove_if(objects.begin(), objects.end(),
                     [&](std::shared_ptr<Object> o) { return obj == o; }),
      objects.end());
}

void Scene::removeObjectsByName(std::string name) {
  objects.erase(std::remove_if(
                    objects.begin(), objects.end(),
                    [&](std::shared_ptr<Object> o) { return o->name == name; }),
                objects.end());
}

void Scene::setMainCamera(const std::shared_ptr<Camera> cam) {
  if (contains(cam)) {
    mainCamera = cam;
  }
}

std::shared_ptr<Camera> Scene::getMainCamera() const { return mainCamera; }

const std::vector<std::shared_ptr<Object>> &Scene::getObjects() const {
  return objects;
}

void Scene::addPointLight(PointLight light) { pointLights.push_back(light); }
void Scene::addDirectionalLight(DirectionalLight light) {
  directionalLights.push_back(light);
}
void Scene::addParalleloGramLight(ParallelogramLight light) {
  parallelogramLights.push_back(light);
}

void Scene::setEnvironmentMap(const std::string &front, const std::string &back,
                              const std::string &top, const std::string &bottom,
                              const std::string &left, const std::string &right,
                              int wrapping, int filtering) {
  environmentMap = LoadCubeMapTexture(front, back, top, bottom, left, right,
                                      wrapping, filtering);
  environmentCube = NewEnvironmentCube();
}
} // namespace Optifuser
