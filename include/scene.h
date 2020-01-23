#pragma once
#include "lights.h"
#include "object.h"
#include <vector>
namespace Optifuser {
class Scene {
public:
  Scene(){};
  ~Scene(){};

private:
  std::vector<std::unique_ptr<Object>> objects;

  std::vector<PointLight> pointLights;
  std::vector<DirectionalLight> directionalLights;
  std::vector<ParallelogramLight> parallelogramLights;
  glm::vec3 ambientLight = {0, 0, 0};
  std::shared_ptr<CubeMapTexture> environmentMap;

  std::vector<std::tuple<glm::vec3, glm::quat, glm::vec3>> axes;

public:
  void addAxes(const glm::vec3 &pos, const glm::quat &rot, const glm::vec3 &scale = {1, 1, 1}) {
    axes.push_back({pos, rot, scale});
  }
  void clearAxes() { axes.clear(); }
  inline const std::vector<std::tuple<glm::vec3, glm::quat, glm::vec3>> &getAxes() const {
    return axes;
  }

public:
  void addObject(std::unique_ptr<Object> obj);
  void removeObject(Object *obj);
  void removeObjectsByName(std::string name);
  const std::vector<std::unique_ptr<Object>> &getObjects() const;

  void setAmbientLight(glm::vec3 light);
  void setShadowLight(DirectionalLight light);
  void addPointLight(PointLight light);
  void addDirectionalLight(DirectionalLight light);
  void addParalleloGramLight(ParallelogramLight light);

  void setEnvironmentMap(const std::string &nz, const std::string &pz, const std::string &py,
                         const std::string &ny, const std::string &nx, const std::string &px,
                         int wrapping = GL_CLAMP_TO_EDGE, int filtering = GL_LINEAR);

  inline const std::shared_ptr<CubeMapTexture> &getEnvironmentMap() const {
    return environmentMap;
  }

  glm::vec3 getAmbientLight() const { return ambientLight; }
  const std::vector<PointLight> &getPointLights() const { return pointLights; }
  const std::vector<DirectionalLight> &getDirectionalLights() const { return directionalLights; }
  const std::vector<ParallelogramLight> &getParallelogramLights() const {
    return parallelogramLights;
  }
};

} // namespace Optifuser
