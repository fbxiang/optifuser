#pragma once
#include "camera_spec.h"
#include "scene.h"
#include <GL/glew.h>

namespace Optifuser {

class ShadowPass {

private:
  GLuint m_fbo;
  GLuint m_shadowtex;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

  int m_width, m_height;

  bool m_initialized;

  int m_frustum_size = 10.f;

public:
  void init();
  void setFrustumSize(int size);
  void setFbo(GLuint fbo);
  void setShader(const std::string &vs, const std::string &fs);
  void setDepthAttachment(GLuint depthtex, int with, int height);
  void bindAttachments() const;
  void render(const Scene &scene, const CameraSpec &camera) const;
};

} // namespace Optifuser
