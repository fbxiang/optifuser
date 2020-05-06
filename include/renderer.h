#pragma once
#include "camera_spec.h"
#include "passes/ao_pass.h"
#include "passes/axis_pass.h"
#include "passes/composite_pass.h"
#include "passes/gbuffer_pass.h"
#include "passes/lighting_pass.h"
#include "passes/shadow_pass.h"
#include "passes/transparency_pass.h"
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
  AO,
  LIGHTING,
  TRANSPARENCY,
  AXIS,
  COMPOSITE,
  DISPLAY,
  COPY,

  COUNT
};

class Renderer {

private:
  std::unique_ptr<ShadowPass> shadow_pass = nullptr;
  std::unique_ptr<GBufferPass> gbuffer_pass = nullptr;
  std::unique_ptr<AOPass> ao_pass = nullptr;
  std::unique_ptr<LightingPass> lighting_pass = nullptr;
  std::unique_ptr<AxisPass> axis_pass = nullptr;
  std::unique_ptr<TransparencyPass> transparency_pass = nullptr;
  std::unique_ptr<CompositePass> composite_pass = nullptr;
  std::unique_ptr<CompositePass> display_pass = nullptr;

  bool shadowPassEnabled = false;
  bool aoPassEnabled = false;
  bool axisPassEnabled = false;
  bool displayPassEnabled = false;

  // Screen-specific factor, depending on DPI setting
  uint8_t scaling = 1;

public:
  GLuint colortex[N_COLORTEX];
  GLuint aotex = 0;
  GLuint depthtex = 0;
  GLuint outputtex = 0;
  GLuint lightingtex = 0;

  GLuint lightingtex2 = 0; // use for composite

  GLuint segtex[3];
  GLuint usertex[1];
  GLuint shadowtex = 0;

  GLuint m_fbo[FBO_TYPE::COUNT];

  GLuint pickingFbo = 0;

  std::shared_ptr<Texture> randomtex;

  void deleteTextures();
  void initTextures();
  void rebindTextures();

public:
  Renderer();
  void init(float scaling = 1);
  void exit();
  void resize(GLuint w, GLuint h);

  int pickSegmentationId(int x, int y);
  int pickObjectId(int x, int y);
  void enablePicking();
  void enableAxisPass(bool enable = true);
  void enableDisplayPass(bool enable = true);
  void enableGlobalAxes(bool enable = true);

  void enableShadowPass(bool enable = true, int shadowmapSize = 2048, float shadowFrustumSize = 10.f);
  void enableAOPass(bool enable = true);

public:
  bool initialized;

public:
public:
  void setAxisShader(const std::string &vs, const std::string &fs);
  void setGBufferShader(const std::string &vs, const std::string &fs);
  void setAOShader(const std::string &vs, const std::string &fs);
  void setDeferredShader(const std::string &vs, const std::string &fs);
  void setShadowShader(const std::string &vs, const std::string &fs);
  void setTransparencyShader(const std::string &vs, const std::string &fs);
  void setCompositeShader(const std::string &vs, const std::string &fs);
  void setDisplayShader(const std::string &vs, const std::string &fs);

  void setObjectIdForAxis(int id);

protected:
  GLuint m_width, m_height;
  GLuint m_shadowSize = 0;
  float m_shadowFrustumSize = 10.f;

public:
  inline GLuint getWidth() const { return m_width; }
  inline GLuint getHeight() const { return m_height; }

public:
  void renderScene(Scene &scene, const CameraSpec &camera);
  void displayLighting(GLuint fbo = 0) const;
  void displaySegmentation(GLuint fbo = 0) const;
  void displayUserTexture(GLuint fbo = 0) const;
  void display(GLuint fbo = 0) const;

  std::vector<float> getLighting();
  std::vector<float> getAlbedo();
  std::vector<float> getNormal();
  std::vector<float> getDepth();
  std::vector<int> getSegmentation();
  std::vector<int> getSegmentation2();
  std::vector<float> getUserTexture();

  void reloadShaders();
};

} // namespace Optifuser
