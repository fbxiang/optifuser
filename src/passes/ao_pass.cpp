#include "passes/ao_pass.h"
#include <iostream>

namespace Optifuser {

AOPass::AOPass() {}
AOPass::~AOPass() {
  glDeleteBuffers(1, &m_quadVbo);
  glDeleteVertexArrays(1, &m_quadVao);
}

void AOPass::init() {
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

void AOPass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
  if (!m_shader) {
    std::cerr << "Composite Shader Creation Failed." << std::endl;
  }
}


void AOPass::setFbo(GLuint fbo) {
  m_fbo = fbo;
}

void AOPass::setAttachment(GLuint texture, int width, int height) {
  m_width = width;
  m_height = height;

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, texture);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
}

void AOPass::setInputTextures(int count, GLuint *colortex, GLuint depthtex) {
  m_colorTextures.resize(0);
  m_colorTextures.insert(m_colorTextures.begin(), colortex, colortex + count);
  m_depthTexture = depthtex;
}

void AOPass::setRandomTexture(GLuint randomtex, int width, int height) {
  m_randomtex = randomtex;
  m_randomtexWidth = width;
  m_randomtexHeight = height;
}

void AOPass::render(const CameraSpec &camera) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);
  glDisable(GL_DEPTH_TEST);
  m_shader->use();

  for (size_t n = 0; n < m_colorTextures.size(); n++) {
    m_shader->setTexture("colortex" + std::to_string(n), m_colorTextures[n], n);
  }
  m_shader->setTexture("depthtex0", m_depthTexture, m_colorTextures.size());

  if (m_randomtex) {
    m_shader->setTexture("randomtex", m_randomtex, m_colorTextures.size() + 1);
    m_shader->setInt("randomtexWidth", m_randomtexWidth);
    m_shader->setInt("randomtexHeight", m_randomtexHeight);
  }
  m_shader->setInt("viewWidth", m_width);
  m_shader->setInt("viewHeight", m_height);

  glm::mat4 projMat = camera.getProjectionMat();
  m_shader->setMatrix("gbufferProjectionMatrix", projMat);
  m_shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));

  // render quad
  glBindVertexArray(m_quadVao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Optifuser
