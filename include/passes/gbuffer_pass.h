#pragma once
#include "camera_spec.h"
#include "scene.h"
#include <GL/glew.h>

namespace Optifuser {

class GBufferPass {

public:
  GBufferPass();

protected:
  GLuint m_fbo;
  std::vector<GLuint> m_colortex;
  GLuint m_depthtex;

  std::string m_vertFile;
  std::string m_fragFile;
  std::shared_ptr<Shader> m_shader;

  int m_width, m_height;

  bool m_initialized;

  bool m_clearDepth = true;

public:
  void init();
  void setFbo(GLuint fbo);
  void setShader(const std::string &vs, const std::string &fs);
  void setColorAttachments(int num, GLuint *tex, int width, int height);
  void setDepthAttachment(GLuint depthtex, bool clear = true);
  void bindAttachments() const;
  void render(const Scene &scene, const CameraSpec &camera,
              bool renderSegmentation = false) const;

  int numColorAttachments() const;

};

} // namespace Optifuser
