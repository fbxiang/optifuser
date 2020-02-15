#include "passes/axis_pass.h"

namespace Optifuser {

void renderGlobalAxis(const glm::mat4 modelMat, const glm::mat4 &viewMat, const glm::mat4 &projMat,
                      Shader *shader, float length = 0.1, float thickness = 0.005) {

  static std::unique_ptr<Object> x = nullptr;
  static std::unique_ptr<Object> y = nullptr;
  static std::unique_ptr<Object> z = nullptr;

  shader->setMatrix("gbufferViewMatrix", viewMat);
  // shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
  shader->setMatrix("gbufferProjectionMatrix", projMat);
  // shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));

  if (!x) {
    x = NewCube();
    y = NewCube();
    z = NewCube();
  }

  x->scale = {length, thickness, thickness};
  x->position = {length, 0, 0};
  y->scale = {thickness, length, thickness};
  y->position = {0, length, 0};
  z->scale = {thickness, thickness, length};
  z->position = {0, 0, length};

  glm::mat mat = modelMat * x->getModelMat();
  shader->setVec3("color", {1, 0, 0});
  shader->setMatrix("gbufferModelMatrix", mat);
  // shader->setMatrix("gbufferModelMatrix", glm::inverse(mat));
  x->getMesh()->draw();

  mat = modelMat * y->getModelMat();
  shader->setVec3("color", {0, 1, 0});
  shader->setMatrix("gbufferModelMatrix", mat);
  // shader->setMatrix("gbufferModelMatrixInverse", glm::inverse(mat));
  y->getMesh()->draw();

  mat = modelMat * z->getModelMat();
  shader->setVec3("color", {0, 0, 1});
  shader->setMatrix("gbufferModelMatrix", mat);
  // shader->setMatrix("gbufferModelMatrixInverse", glm::inverse(mat));
  z->getMesh()->draw();
}

void renderObjectTree(const Object &obj, const glm::mat4 &parentModelMat, const glm::mat4 &viewMat,
                      const glm::mat4 &viewMatInv, const glm::mat4 &projMat,
                      const glm::mat4 &projMatInv, Shader *shader) {

  glm::mat4 modelMat = parentModelMat * obj.getModelMat();
  auto mesh = obj.getMesh();
  if (obj.showAxis) {
    shader->setMatrix("gbufferViewMatrix", viewMat);
    // shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
    shader->setMatrix("gbufferProjectionMatrix", projMat);
    // shader->setMatrix("gbufferProjectionMatrixInverse", glm::inverse(projMat));

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

    glm::mat mat = modelMat * x->getModelMat();
    shader->setVec3("color", {1, 0, 0});
    shader->setMatrix("gbufferModelMatrix", mat);
    // shader->setMatrix("gbufferModelMatrix", glm::inverse(mat));
    x->getMesh()->draw();

    mat = modelMat * y->getModelMat();
    shader->setVec3("color", {0, 1, 0});
    shader->setMatrix("gbufferModelMatrix", mat);
    // shader->setMatrix("gbufferModelMatrixInverse", glm::inverse(mat));
    y->getMesh()->draw();

    mat = modelMat * z->getModelMat();
    shader->setVec3("color", {0, 0, 1});
    shader->setMatrix("gbufferModelMatrix", mat);
    // shader->setMatrix("gbufferModelMatrixInverse", glm::inverse(mat));
    z->getMesh()->draw();
  }
  for (auto &child : obj.getChildren()) {
    renderObjectTree(*child, modelMat, viewMat, viewMatInv, projMat, projMatInv, shader);
  }
}

void AxisPass::render(const Scene &scene, const CameraSpec &camera) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // do not clear color or depth
  glm::mat4 viewMat = camera.getViewMat();
  // glm::mat4 viewMatInv = glm::inverse(viewMat);
  glm::mat4 projMat = camera.getProjectionMat();
  // glm::mat4 projMatInv = glm::inverse(projMat);

  m_shader->use();

  for (auto &[pos, rot, scale] : scene.getAxes()) {
    glm::mat4 t = glm::toMat4(rot);
    t[3][0] = pos.x;
    t[3][1] = pos.y;
    t[3][2] = pos.z;
    t[0][0] *= scale.x;
    t[1][1] *= scale.y;
    t[2][2] *= scale.z;

    renderGlobalAxis(t, viewMat, projMat, m_shader.get());
  }
  renderGlobalAxis(glm::mat4(1), viewMat, projMat, m_shader.get(), 1, 0.01);
}

} // namespace Optifuser
