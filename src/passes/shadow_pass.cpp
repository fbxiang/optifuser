#include "passes/shadow_pass.h"
#include "debug.h"

namespace Optifuser {

void ShadowPass::init() { m_initialized = true; }

void ShadowPass::setFrustumSize(int size) {
  m_frustum_size = size;
}

void ShadowPass::setFbo(GLuint fbo) {
  m_fbo = fbo;
  LABEL_FRAMEBUFFER(fbo, "Shadow FBO");
}

void ShadowPass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
}

void ShadowPass::setDepthAttachment(GLuint depthtex, int width, int height) {
  m_width = width;
  m_height = height;
  m_shadowtex = depthtex;
  bindAttachments();
}

void ShadowPass::bindAttachments() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, m_shadowtex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_shadowtex, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// helper method for rendering an object tree
void renderObjectTree(const Object &obj, Shader *shader) {

  glm::mat4 modelMat = obj.globalModelMatrix;
  auto mesh = obj.getMesh();
  if (mesh && obj.visibility > 0.f) {
    shader->setMatrix("gbufferModelMatrix", modelMat);
    mesh->draw();
  }
}

void ShadowPass::render(const Scene &scene, const CameraSpec &camera) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);
  m_shader->use();

  if (scene.getDirectionalLights().empty()) {
    return;
  }

  glm::vec3 dir = scene.getDirectionalLights()[0].direction;
  // glm::mat4 w2c  = camera.getViewMat();
  glm::mat4 w2c = glm::translate(glm::mat4(1), -camera.position);

  glm::vec3 csLightDir = w2c * glm::vec4(dir, 0);
  glm::mat4 c2l =
      glm::lookAt(glm::vec3(0, 0, 0), csLightDir, glm::vec3(0, 1, 0));

  float v = m_frustum_size;
  glm::mat4 lsProj = glm::ortho(-v, v, -v, v, -v, camera.far);

  glm::mat4 w2l = c2l * w2c;
  glm::mat4 lightSpaceMatrix = lsProj * w2l;

  m_shader->setMatrix("lightSpaceMatrix", lightSpaceMatrix);

  for (const auto &obj : scene.getOpaqueObjects()) {
    renderObjectTree(*obj, m_shader.get());
  }
}

} // namespace Optifuser
