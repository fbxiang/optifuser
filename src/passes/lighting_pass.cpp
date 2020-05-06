#include "passes/lighting_pass.h"
#include "debug.h"

namespace Optifuser {

LightingPass::LightingPass() {}

LightingPass::~LightingPass() {
  glDeleteBuffers(1, &m_quadVbo);
  glDeleteVertexArrays(1, &m_quadVao);
}

void LightingPass::setShadowFrustumSize(int size) { m_shadow_frustum_size = size; }

void LightingPass::setFbo(GLuint fbo) {
  m_fbo = fbo;
  LABEL_FRAMEBUFFER(fbo, "Lighting FBO");
};

void LightingPass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
}

void LightingPass::init() {
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

void LightingPass::setAttachment(GLuint texture, int width, int height) {
  m_width = width;
  m_height = height;

  // bind texture
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, texture);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
}

void LightingPass::setInputTextures(int count, GLuint *colortex, GLuint depthtex) {
  m_colorTextures.resize(0);
  m_colorTextures.insert(m_colorTextures.begin(), colortex, colortex + count);
  m_depthTexture = depthtex;
}

void LightingPass::setShadowTexture(GLuint shadowtex, int size) {
  m_shadowtex = shadowtex;
  m_shadowtex_size = size;
}

void LightingPass::setAOTexture(GLuint aotex) { m_aotex = aotex; }

void LightingPass::setRandomTexture(GLuint randomtex, GLuint width, GLuint height) {
  m_randomtex = randomtex;
  m_randomtex_width = width;
  m_randomtex_height = height;
}

void LightingPass::render(const Scene &scene, const CameraSpec &camera) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);

  glDisable(GL_DEPTH_TEST);
  m_shader->use();
  glm::mat4 viewMat = camera.getViewMat();
  glm::mat4 projMat = camera.getProjectionMat();

  // set uniforms
  m_shader->setMatrix("gbufferViewMatrix", viewMat);
  m_shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
  m_shader->setMatrix("gbufferProjectionMatrix", projMat);
  m_shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));
  m_shader->setMatrix("environmentViewMatrix", viewMat);
  m_shader->setMatrix("environmentViewMatrixInverse", glm::inverse(viewMat));
  m_shader->setVec3("ambientLight", scene.getAmbientLight());

  // point lights
  const auto &pointLights = scene.getPointLights();
  for (size_t i = 0; i < pointLights.size(); i++) {
    std::string p = "pointLights[" + std::to_string(i) + "].position";
    std::string e = "pointLights[" + std::to_string(i) + "].emission";
    m_shader->setVec3(p, pointLights[i].position);
    m_shader->setVec3(e, pointLights[i].emission);
  }

  // directional lights
  const auto &directionalLights = scene.getDirectionalLights();
  for (size_t i = 0; i < directionalLights.size(); i++) {
    std::string d = "directionalLights[" + std::to_string(i) + "].direction";
    std::string e = "directionalLights[" + std::to_string(i) + "].emission";
    m_shader->setVec3(d, directionalLights[i].direction);
    m_shader->setVec3(e, directionalLights[i].emission);
  }

  for (size_t n = 0; n < m_colorTextures.size(); n++) {
    m_shader->setTexture("colortex" + std::to_string(n), m_colorTextures[n], n);
  }
  m_shader->setTexture("depthtex0", m_depthTexture, m_colorTextures.size());
  if (auto envmap = scene.getEnvironmentMap()) {
    m_shader->setCubemap("skybox", scene.getEnvironmentMap()->getId(), m_colorTextures.size() + 1);
  }

  if (m_aotex) {
    m_shader->setTexture("aotex", m_aotex, m_colorTextures.size() + 4);
    m_shader->setBool("aoEnabled", true);
  } else {
    m_shader->setBool("aoEnabled", false);
  }

  // randomtex
  m_shader->setTexture("randomtex", m_randomtex, m_colorTextures.size() + 3);
  m_shader->setInt("randomtexWidth", m_randomtex_width);
  m_shader->setInt("randomtexHeight", m_randomtex_height);
  m_shader->setInt("viewWidth", m_width);
  m_shader->setInt("viewHeight", m_height);

  if (m_shadowtex && directionalLights.size()) {
    glm::vec3 dir = directionalLights[0].direction;

    glm::mat4 w2c = glm::translate(glm::mat4(1), -camera.position);
    glm::vec3 csLightDir = w2c * glm::vec4(dir, 0);
    glm::mat4 c2l = glm::lookAt(glm::vec3(0, 0, 0), csLightDir, glm::vec3(0, 1, 0));

    float v = m_shadow_frustum_size;
    glm::mat4 lsProj = glm::ortho(-v, v, -v, v, -v, camera.far);

    m_shader->setTexture("shadowtex", m_shadowtex, m_colorTextures.size() + 2);
    m_shader->setMatrix("cameraToShadowMatrix", c2l * glm::toMat4(camera.getRotation()));
    m_shader->setMatrix("shadowProjectionMatrix", lsProj);
    m_shader->setBool("shadowLightEnabled", true);
    m_shader->setInt("shadowtexSize", m_shadowtex_size);
  } else {
    m_shader->setBool("shadowLightEnabled", false);
  }

  if (directionalLights.size()) {
    glm::vec3 dir = directionalLights[0].direction;
    glm::vec3 em = directionalLights[0].emission;
    m_shader->setVec3("shadowLightDirection", dir);
    m_shader->setVec3("shadowLightEmission", em);
  }

  // render quad
  glBindVertexArray(m_quadVao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

} // namespace Optifuser
