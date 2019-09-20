#include "passes/axis_pass.h"

namespace Optifuser {

void renderObjectTree(const Object &obj, const glm::mat4 &parentModelMat,
                      const glm::mat4 &viewMat, const glm::mat4 &viewMatInv,
                      const glm::mat4 &projMat, const glm::mat4 &projMatInv,
                      Shader *shader, int id) {

  glm::mat4 modelMat = parentModelMat * obj.getModelMat();
  auto mesh = obj.getMesh();
  if (obj.getSegmentId() == id) {
    shader->setMatrix("gbufferViewMatrix", viewMat);
    shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
    shader->setMatrix("gbufferProjectionMatrix", projMat);
    shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));

    static std::unique_ptr<Object> x = nullptr;
    static std::unique_ptr<Object> y = nullptr;
    static std::unique_ptr<Object> z = nullptr;
    if (!x) {
      float t = 0.1;
      x = NewCube();
      x->scale = {1, 0.05, 0.05};
      x->position = {1, 0, 0};
      x->scale *= t;
      x->position *= t;

      y = NewCube();
      y->scale = {0.05, 1, 0.05};
      y->position = {0, 1, 0};
      y->scale *= t;
      y->position *= t;

      z = NewCube();
      z->scale = {0.05, 0.05, 1};
      z->position = {0, 0, 1};
      z->scale *= t;
      z->position *= t;
    }
    shader->setVec3("color", {1, 0, 0});
    shader->setMatrix("gbufferModelMatrix", modelMat * x->getModelMat());
    x->getMesh()->draw();

    shader->setVec3("color", {0, 1, 0});
    shader->setMatrix("gbufferModelMatrix", modelMat * y->getModelMat());
    y->getMesh()->draw();

    shader->setVec3("color", {0, 0, 1});
    shader->setMatrix("gbufferModelMatrix", modelMat * z->getModelMat());
    z->getMesh()->draw();
  }
  for (auto &child : obj.getChildren()) {
    renderObjectTree(*child, modelMat, viewMat, viewMatInv, projMat, projMatInv,
                     shader, id);
  }
}

void AxisPass::render(const Scene &scene, const CameraSpec &camera) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // do not clear color or depth
  glm::mat4 viewMat = camera.getViewMat();
  glm::mat4 viewMatInv = glm::inverse(viewMat);
  glm::mat4 projMat = camera.getProjectionMat();
  glm::mat4 projMatInv = glm::inverse(projMat);

  m_shader->use();
  for (const auto &obj : scene.getObjects()) {
    renderObjectTree(*obj, glm::mat4(1.f), viewMat, viewMatInv, projMat,
                     projMatInv, m_shader.get(), objId);
  }
}

} // namespace Optifuser
