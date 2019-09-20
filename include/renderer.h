#pragma once
#include "camera_spec.h"
#include "passes/axis_pass.h"
#include "passes/gbuffer_pass.h"
#include "passes/lighting_pass.h"
#include "passes/shadow_pass.h"
#include "scene.h"
#include "shader.h"
#include <GL/glew.h>
#include <map>
#include <stdint.h>

#define N_COLORTEX 3
namespace Optifuser {

enum FBO_TYPE {
  SHADOW,
  GBUFFER,
  LIGHTING,
  AXIS,
  COPY,

  COUNT
};

class Renderer {
private:
  ShadowPass shadow_pass;
  GBufferPass gbuffer_pass;
  LightingPass lighting_pass;
  AxisPass axis_pass;

public:
  GLuint colortex[N_COLORTEX];
  GLuint depthtex = 0;
  GLuint outputtex = 0;
  GLuint segtex[2];
  GLuint shadowtex = 0;

  GLuint m_fbo[FBO_TYPE::COUNT];

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
  void setAxisShader(const std::string &vs, const std::string &fs);
  void setGBufferShader(const std::string &vs, const std::string &fs);
  void setDeferredShader(const std::string &vs, const std::string &fs);
  void setShadowShader(const std::string &vs, const std::string &fs);

  void setObjectIdForAxis(int id);

protected:
  GLuint m_width, m_height;

  GLuint shadowWidth = 8192;
  GLuint shadowHeight = 8192;

public:
  inline GLuint getWidth() const { return m_width; }
  inline GLuint getHeight() const { return m_height; }

public:
  void renderScene(const Scene &scene, const CameraSpec &camera);
  void displayLighting(GLuint fbo = 0) const;
  void displaySegmentation(GLuint fbo = 0) const;

  void saveLighting(const std::string &file, bool raw = true);
  void saveNormal(const std::string &file, bool raw = true);
  void saveDepth(const std::string &file, bool raw = true);
  // void saveSegmentation(const std::string &file);

  void reloadShaders();
};

} // namespace Optifuser
