#include "passes/gbuffer_pass.h"
#include "debug.h"
#include <glm/glm.hpp>
#include <iostream>

namespace Optifuser {

GBufferPass::GBufferPass()
    : m_fbo(0), m_depthtex(0), m_width(0), m_height(0), m_initialized(false) {}

void GBufferPass::init() { m_initialized = true; }

void GBufferPass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
  if (!m_shader) {
    std::cerr << "GBuffer Shader Creation Failed." << std::endl;
  }
}

void GBufferPass::setFbo(GLuint fbo) {
  m_fbo = fbo;
  LABEL_FRAMEBUFFER(fbo, "GBuffer FBO");
}

void GBufferPass::setColorAttachments(int num, GLuint *tex, int width,
                                      int height) {
  m_width = width;
  m_height = height;
  m_colortex.resize(0);
  m_colortex.insert(m_colortex.begin(), tex, tex + num);
}

void GBufferPass::setDepthAttachment(GLuint depthtex) { m_depthtex = depthtex; }

// helper method for rendering an object tree
void renderObjectTree(const Object &obj, const glm::mat4 &parentModelMat,
                      const glm::mat4 &viewMat, const glm::mat4 &viewMatInv,
                      const glm::mat4 &projMat, const glm::mat4 &projMatInv,
                      Shader *defaultShader) {

  glm::mat4 modelMat = parentModelMat * obj.getModelMat();
  auto mesh = obj.getMesh();
  if (mesh) {
    auto shader = obj.shader.get();
    if (shader) {
      shader->use();
    } else {
      shader = defaultShader;
      shader->use();
    }
    shader->setMatrix("gbufferViewMatrix", viewMat);
    shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
    shader->setMatrix("gbufferProjectionMatrix", projMat);
    shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));

    shader->setMatrix("gbufferModelMatrix", modelMat);
    shader->setVec3("material.ka", obj.material.ka);
    shader->setVec3("material.kd", obj.material.kd);
    shader->setVec3("material.ks", obj.material.ks);
    shader->setFloat("material.ke", obj.material.exp);
    shader->setTexture("material.kd_map", obj.material.kd_map->getId(), 0);
    shader->setBool("material.has_kd_map", obj.material.kd_map->getId() != 0);
    shader->setTexture("material.ks_map", obj.material.ks_map->getId(), 1);
    shader->setBool("material.has_ks_map", obj.material.ks_map->getId() != 0);
    shader->setTexture("material.height_map", obj.material.height_map->getId(),
                       2);
    shader->setBool("material.has_height_map",
                    obj.material.height_map->getId() != 0);
    shader->setTexture("material.normal_map", obj.material.normal_map->getId(),
                       3);
    shader->setBool("material.has_normal_map",
                    obj.material.normal_map->getId() != 0);

    mesh->draw();
  }
  for (auto &child : obj.getChildren()) {
    renderObjectTree(*child, modelMat, viewMat, viewMatInv, projMat, projMatInv,
                     defaultShader);
  }
}

void GBufferPass::render(const Scene &scene, const CameraSpec &camera) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);

  if (m_depthtex) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4 viewMat = camera.getViewMat();
  glm::mat4 viewMatInv = glm::inverse(viewMat);
  glm::mat4 projMat = camera.getProjectionMat();

  glm::mat4 projMatInv = glm::inverse(projMat);

  // TODO: render axes
  for (const auto &obj : scene.getObjects()) {
    renderObjectTree(*obj, glm::mat4(1.f), viewMat, viewMatInv, projMat,
                     projMatInv, m_shader.get());
  }
}

int GBufferPass::numColorAttachments() const { return m_colortex.size(); }

void GBufferPass::bindAttachments() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  int count = numColorAttachments();
  GLuint attachments[count];
  for (int n = 0; n < count; ++n) {
    glBindTexture(GL_TEXTURE_2D, m_colortex[n]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                           GL_TEXTURE_2D, m_colortex[n], 0);
    attachments[n] = GL_COLOR_ATTACHMENT0 + n;
  }
  glDrawBuffers(count, attachments);

  glBindTexture(GL_TEXTURE_2D, m_depthtex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_depthtex, 0);
}

} // namespace Optifuser
