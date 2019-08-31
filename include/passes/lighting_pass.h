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
  bool m_initialized;

  GLuint m_fbo;
  GLuint m_quadVao;
  GLuint m_quadVbo;

  int m_numColorTextures;
  GLuint *m_colorTextures;
  GLuint m_depthTexture;

  int m_width, m_height;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

public:
  void init();
  void setShader(const std::string &vs, const std::string &fs);
  void setAttachment(GLuint texture, int width, int height);

  void setFbo(GLuint fbo);
  void setInputTextures(int count, GLuint *colortex, GLuint depthtex);
  void render(const Scene &scene, const CameraSpec &camera) const;
};

} // namespace Optifuser
