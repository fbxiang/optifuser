#pragma once
#include "camera_spec.h"
#include "passes/gbuffer_pass.h"
#include "passes/lighting_pass.h"
#include "scene.h"
#include "shader.h"
#include <GL/glew.h>
#include <map>
#include <stdint.h>

#define N_COLORTEX  3
#define N_FBO 3
namespace Optifuser {
class Renderer {
private:
  GBufferPass gbuffer_pass;
  LightingPass lighting_pass;

  GLuint colortex[N_COLORTEX];
  GLuint depthtex = 0;
  GLuint outputtex = 0;
  GLuint segtex[2];

  GLuint m_fbo[N_FBO];

  void deleteTextures();
  void initTextures();
  void rebindTextures();

  bool m_renderSegmentation = false;

public:
  int debug = 0;

  float worldAxesScale = 0;
  float objectAxesScale = 0;

public:
  Renderer();
  void init();
  void exit();
  void resize(GLuint w, GLuint h);

  void renderSegmentation(bool enabled = true);

public:
  bool initialized;

public:
public:
  void setGBufferShader(const std::string &vs, const std::string &fs);
  void setDeferredShader(const std::string &vs, const std::string &fs);

protected:
  GLuint m_width, m_height;

public:
  inline GLuint getWidth() const { return m_width; }
  inline GLuint getHeight() const { return m_height; }

public:
  void renderScene(const Scene &scene, const CameraSpec &camera);
  void displayLighting(GLuint fbo = 0) const;
  void displaySegmentation(GLuint fbo = 0) const;
  void displayDepth(GLuint fbo = 0) const;
  void reloadShaders();
};

} // namespace Optifuser
