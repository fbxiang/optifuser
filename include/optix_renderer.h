#pragma once
#include "scene.h"
#include "shader.h"
#include <GL/glew.h>
#include <camera_spec.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <optixu/optixpp.h>
#include <optixu/optixu_math_namespace.h>

namespace Optifuser {

class OptixRenderer {
public:
  OptixRenderer();
  ~OptixRenderer();
  void init(uint32_t w, uint32_t h);
  void exit();

  void invalidateCamera() { iterations = 0; };
  uint32_t max_iterations = 64;

private:
  std::map<const Object *, optix::Transform> _object_transform;
  std::map<const TriangleMesh *, optix::Geometry> _mesh_geometry;
  std::map<const DynamicMesh *, optix::Geometry> _dmesh_geometry;
  std::map<const Object *, optix::Acceleration> _object_accel;
  std::map<const Texture *, optix::TextureSampler> _texture_sampler;
  optix::TextureSampler _empty_sampler = 0;

  optix::Program _dmesh_intersect = 0;
  optix::Program _dmesh_bounds = 0;

  optix::Program _mesh_intersect = 0;
  optix::Program _mesh_bounds = 0;
  optix::Program _material_closest_hit = 0;
  optix::Program _material_any_hit = 0;
  optix::Program _material_shadow_any_hit = 0;
  optix::Program _material_fluid_closest_hit = 0;
  optix::Program _material_fluid_any_hit = 0;
  optix::Program _material_fluid_shadow_any_hit = 0;
  optix::Program _material_mirror_closest_hit = 0;
  optix::Program _material_mirror_any_hit = 0;
  optix::Program _material_mirror_shadow_any_hit = 0;

  optix::Transform getObjectTransform(const Object *obj);
  optix::Geometry getMeshGeometry(const TriangleMesh *mesh);
  optix::Geometry getMeshGeometry(const DynamicMesh *mesh);
  optix::Acceleration getObjectAccel(const Object *obj);
  optix::TextureSampler getTextureSampler(const Texture *tex);
  optix::TextureSampler getEmptySampler();

  uint32_t iterations = 0;

private:
  enum BackgroundMode { BLACK, PROCEDUAL_SKY, CUBEMAP, HDRMAP } backgroundMode = PROCEDUAL_SKY;

  bool sceneInitialized = false;
  void initSceneGeometry(const Scene &scene);
  void initSceneLights(const Scene &scene);

public:
  uint32_t numRays = 1;
  bool useShadow = 1;

public:
  inline uint32_t getWidth() const { return width; }
  inline uint32_t getHeight() const { return height; }
  inline void resize(int width, int height) { std::cerr << "resize not implemented" << std::endl; }

  GLuint outputTex = 0;

private:
  uint32_t width, height;
  bool initialized = false;

  optix::Context context = 0;
  GLuint transferFbo = 0;
  GLuint screenVbo = 0;

  uint32_t nSamplesSqrt = 1;

  struct Cubemap {
    uint32_t width;
    std::vector<unsigned char> front;
    std::vector<unsigned char> back;
    std::vector<unsigned char> left;
    std::vector<unsigned char> right;
    std::vector<unsigned char> top;
    std::vector<unsigned char> bottom;
  } cubemap;

  struct Hdrmap {
    uint32_t height;
    std::vector<float> texture;
  } hdrmap;

public:
  void renderScene(const Scene &scene, const CameraSpec &camera);
  void display();
  std::vector<float> getResult();
  void renderSceneToFile(const Scene &scene, const CameraSpec &cam, std::string filename);
  // void renderCurrentToFile(std::string filename);
  void setCubemap(std::string const &front, std::string const &back, std::string const &top,
                  std::string const &bottom, std::string const &left, std::string const &right);

  void setHdrmap(std::string const &map);
  inline void setBlackBackground() { backgroundMode = BLACK; };
  inline void setProcedualSkyBackground() { backgroundMode = PROCEDUAL_SKY; };
};

} // namespace Optifuser
