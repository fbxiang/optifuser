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

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

  int m_width, m_height;

  bool m_initialized;

public:
  void init();
  void setFbo(GLuint fbo);
  void setShadowTexture(GLuint shadowtex);
  void setShader(const std::string &vs, const std::string &fs);
  void setColorAttachments(int num, GLuint *tex, int width, int height);
  void setDepthAttachment(GLuint depthtex);
  void bindAttachments() const;
  void render(const Scene &scene, const CameraSpec &camera, bool renderSegmentation = false) const;

  int numColorAttachments() const;
};

} // namespace Optifuser
