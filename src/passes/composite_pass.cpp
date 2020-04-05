#include "passes/composite_pass.h"
#include <iostream>

namespace Optifuser {

CompositePass::CompositePass() {}
CompositePass::~CompositePass() {
  glDeleteBuffers(1, &m_quadVbo);
  glDeleteVertexArrays(1, &m_quadVao);
}

void CompositePass::init() {
  m_initialized = true;
  glGenVertexArrays(1, &m_quadVao);
  glBindVertexArray(m_quadVao);
  glGenBuffers(1, &m_quadVbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
  glEnableVertexAttribArray(0);
  static float vertices[] = {0, 0, 1, 0, 1, 1, 0, 1};
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
}

void CompositePass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
  if (!m_shader) {
    std::cerr << "Composite Shader Creation Failed." << std::endl;
  }
}


void CompositePass::setFbo(GLuint fbo) {
  m_fbo = fbo;
}

void CompositePass::setAttachment(GLuint texture, int width, int height) {
  m_width = width;
  m_height = height;

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, texture);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
}

void CompositePass::setInputTextures(int count, GLuint *colortex, GLuint depthtex) {
  m_colorTextures.resize(0);
  m_colorTextures.insert(m_colorTextures.begin(), colortex, colortex + count);
  m_depthTexture = depthtex;
}

void CompositePass::render() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);
  glDisable(GL_DEPTH_TEST);
  m_shader->use();

  for (size_t n = 0; n < m_colorTextures.size(); n++) {
    m_shader->setTexture("colortex" + std::to_string(n), m_colorTextures[n], n);
  }
  m_shader->setTexture("depthtex0", m_depthTexture, m_colorTextures.size());

  // render quad
  glBindVertexArray(m_quadVao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Optifuser
