#pragma once
#include "camera_spec.h"
#include "scene.h"
#include <GL/glew.h>

namespace Optifuser {

class LightingPass {

public:
  LightingPass();
  ~LightingPass();

private:
  bool m_initialized = false;

  GLuint m_fbo = 0;
  GLuint m_quadVao = 0;
  GLuint m_quadVbo = 0;

  std::vector<GLuint> m_colorTextures;
  GLuint m_depthTexture = 0;
  GLuint m_shadowtex = 0;
  int m_shadowtex_size = 0;

  GLuint m_randomtex = 0;
  int m_randomtex_width = 0;
  int m_randomtex_height = 0;

  GLuint m_aotex = 0;

  int m_width, m_height;
  int m_shadow_frustum_size = 10.f;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

public:
  void init();
  void setShadowFrustumSize(int size);
  void setShader(const std::string &vs, const std::string &fs);
  void setAttachment(GLuint texture, int width, int height);

  void setFbo(GLuint fbo);
  void setInputTextures(int count, GLuint *colortex, GLuint depthtex);
  void setShadowTexture(GLuint shadowtex, int size);
  void setRandomTexture(GLuint randomtex, GLuint width, GLuint height);
  void setAOTexture(GLuint aotex);
  void render(const Scene &scene, const CameraSpec &camera) const;
};

} // namespace Optifuser
