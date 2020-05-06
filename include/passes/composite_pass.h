#pragma once
#include "camera_spec.h"
#include "scene.h"
#include <GL/glew.h>

namespace Optifuser {
class CompositePass {

public:
  CompositePass();
  ~CompositePass();

private:
  bool m_initialized = false;

  GLuint m_fbo = 0;
  GLuint m_quadVao = 0;
  GLuint m_quadVbo = 0;

  std::vector<GLuint> m_colorTextures;
  GLuint m_depthTexture;

  int m_width, m_height;

  GLuint m_randomtex = 0;
  int m_randomtexWidth;
  int m_randomtexHeight;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

public:
  void init();
  void setShader(const std::string &vs, const std::string &fs);
  void setAttachment(GLuint texture, int width, int height);

  void setFbo(GLuint fbo);
  void setInputTextures(int count, GLuint *colortex, GLuint depthtex);
  void setRandomTexture(GLuint randomtex, int width, int height);
  void render() const;
};

} // namespace Optifuser
