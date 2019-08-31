#pragma once
#include "camera_spec.h"
#include "passes/gbuffer_pass.h"
#include "passes/lighting_pass.h"
#include "scene.h"
#include "shader.h"
#include <GL/glew.h>
#include <map>
#include <stdint.h>

#define N_COLOR_ATTACHMENTS 3
#define N_FBO 2
namespace Optifuser {
class Renderer {
private:
  GBufferPass gbuffer_pass;
  LightingPass lighting_pass;

  GLuint colortex[N_COLOR_ATTACHMENTS];
  GLuint depthtex;
  GLuint outputtex;

  GLuint m_fbo[N_FBO];

  void deleteTextures();
  void initTextures(int width, int height);

public:
  int debug = 0;

  float worldAxesScale = 0;
  float objectAxesScale = 0;

public:
  Renderer();
  void init();
  void exit();
  void resize(GLuint w, GLuint h);

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
  void renderScene(const Scene &scene, const CameraSpec &camera,
                   GLuint fbo = 0);
  void reloadShaders();
};

} // namespace Optifuser
