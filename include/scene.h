#pragma once
#include "camera.h"
#include "lights.h"
#include "object.h"
#include <vector>
namespace Optifuser {
class Scene : public std::enable_shared_from_this<Scene> {
public:
  Scene(){};
  ~Scene(){};

private:
  std::vector<std::unique_ptr<Object>> objects;

  std::vector<PointLight> pointLights;
  std::vector<DirectionalLight> directionalLights;
  std::vector<ParallelogramLight> parallelogramLights;
  glm::vec3 ambientLight;
  std::shared_ptr<CubeMapTexture> environmentMap;

public:
  void addObject(std::unique_ptr<Object> obj);
  void removeObject(std::unique_ptr<Object> &obj);
  void removeObjectsByName(std::string name);
  const std::vector<std::unique_ptr<Object>> &getObjects() const;

  void setAmbientLight(glm::vec3 light);
  void addPointLight(PointLight light);
  void addDirectionalLight(DirectionalLight light);
  void addParalleloGramLight(ParallelogramLight light);

  void setEnvironmentMap(const std::string &front, const std::string &back,
                         const std::string &top, const std::string &bottom,
                         const std::string &left, const std::string &right,
                         int wrapping = GL_CLAMP_TO_EDGE,
                         int filtering = GL_LINEAR);

  inline const std::shared_ptr<CubeMapTexture> &getEnvironmentMap() const {
    return environmentMap;
  }

  glm::vec3 getAmbientLight() const { return ambientLight; }
  const std::vector<PointLight> &getPointLights() const { return pointLights; }
  const std::vector<DirectionalLight> &getDirectionalLights() const {
    return directionalLights;
  }
  const std::vector<ParallelogramLight> &getParallelogramLights() const {
    return parallelogramLights;
  }
};

} // namespace Optifuser
