#pragma once
#include "camera_spec.h"
#include "scene.h"
#include <GL/glew.h>

namespace Optifuser {

class TransparencyPass {

public:
  TransparencyPass();

protected:
  GLuint m_fbo;
  std::vector<GLuint> m_colortex;  // lighting
  GLuint m_depthtex;
  GLuint m_shadowtex;
  int m_shadowtex_size = 0;

  GLuint m_randomtex = 0;
  int m_randomtex_width = 0;
  int m_randomtex_height = 0;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

  int m_width, m_height;
  int m_shadow_frustum_size = 10.f;

  bool m_initialized;

public:
  void init();
  void setShadowFrustumSize(int size);
  void setFbo(GLuint fbo);
  void setShadowTexture(GLuint shadowtex, int size);
  void setRandomTexture(GLuint randomtex, GLuint width, GLuint height);
  void setShader(const std::string &vs, const std::string &fs);
  void setColorAttachments(int num, GLuint *tex, int width, int height);
  void setDepthAttachment(GLuint depthtex);
  void bindAttachments() const;
  void render(const Scene &scene, const CameraSpec &camera, bool renderSegmentation = false) const;

  int numColorAttachments() const;
};

} // namespace Optifuser
