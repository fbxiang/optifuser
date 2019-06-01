#include "renderer.h"
namespace Optifuser {
Renderer::Renderer(GLuint w, GLuint h) : width(w), height(h) {}

void Renderer::setGBufferShader(const std::string &vs, const std::string &fs) {
  gbufferVertShaderFile = vs;
  gbufferFragShaderFile = fs;
  gbufferShader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
}
void Renderer::setDeferredShader(const std::string &vs, const std::string &fs) {
  deferredVertShaderFile = vs;
  deferredFragShaderFile = fs;
  deferredShader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
}
void Renderer::setSkyboxShader(const std::string &vs, const std::string &fs) {
  skyboxVertShaderFile = vs;
  skyboxFragShaderFile = fs;
  skyboxShader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
}

void Renderer::init() {
  glViewport(0, 0, width, height);
  glEnable(GL_FRAMEBUFFER_SRGB_EXT);
  initGbufferFramebuffer();
  initColortex();
  initDepthtex();
  bindAttachments();
  initDeferredQuad();
  initCompositeFramebuffer();
  initCompositeTex();
  initialized = true;
}

void Renderer::exit() {
  initialized = false;
  deleteCompositeTex();
  deleteCompositeFramebuffer();
  deleteDeferredQuad();
  deleteDepthtex();
  deleteColortex();
  deleteGbufferFramebuffer();
  gbufferShader = nullptr;
  deferredShader = nullptr;
  skyboxShader = nullptr;
}

void Renderer::resize(GLuint w, GLuint h) {
  exit();
  width = w;
  height = h;
  init();
}

void Renderer::reloadShaders() {
  if (gbufferShader) {
    setGBufferShader(gbufferVertShaderFile, gbufferFragShaderFile);
  }
  if (deferredShader) {
    setDeferredShader(deferredVertShaderFile, deferredFragShaderFile);
  }
  if (skyboxShader) {
    setSkyboxShader(skyboxVertShaderFile, skyboxFragShaderFile);
  }
}

void Renderer::initGbufferFramebuffer() { glGenFramebuffers(1, &g_fbo); }

void Renderer::deleteGbufferFramebuffer() { glDeleteFramebuffers(1, &g_fbo); }

void Renderer::initColortex() {
  for (int n = 0; n < N_COLOR_ATTACHMENTS; n++) {
    glGenTextures(1, &colortex[n]);
    glBindTexture(GL_TEXTURE_2D, colortex[n]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
}

void Renderer::deleteColortex() {
  for (int n = 0; n < N_COLOR_ATTACHMENTS; n++) {
    glDeleteTextures(1, &colortex[n]);
  }
}

void Renderer::initDepthtex() {
  glGenTextures(1, &depthtex);
  glBindTexture(GL_TEXTURE_2D, depthtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
}

void Renderer::deleteDepthtex() { glDeleteTextures(1, &depthtex); }

void Renderer::bindAttachments() {
  glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
  GLuint attachments[N_COLOR_ATTACHMENTS];
  for (int n = 0; n < N_COLOR_ATTACHMENTS; n++) {
    glBindTexture(GL_TEXTURE_2D, colortex[n]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                           GL_TEXTURE_2D, colortex[n], 0);
    attachments[n] = GL_COLOR_ATTACHMENT0 + n;
  }
  glBindTexture(GL_TEXTURE_2D, depthtex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthtex, 0);

  glDrawBuffers(N_COLOR_ATTACHMENTS, attachments);
}

void Renderer::initCompositeFramebuffer() {
  glGenFramebuffers(1, &composite_fbo);
}

void Renderer::deleteCompositeTex() { glDeleteTextures(1, &compositeTex); }

void Renderer::initCompositeTex() {
  glGenTextures(1, &compositeTex);
  glBindFramebuffer(GL_FRAMEBUFFER, composite_fbo);
  glBindTexture(GL_TEXTURE_2D, compositeTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
               GL_FLOAT, NULL);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         compositeTex, 0);
  GLuint attachments[]{GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
}

void Renderer::deleteCompositeFramebuffer() {
  glDeleteFramebuffers(1, &composite_fbo);
}

void Renderer::initDeferredQuad() {
  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);

  glGenBuffers(1, &quadVBO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

  glEnableVertexAttribArray(0);
  static float vertices[] = {0, 0, 1, 0, 1, 1, 0, 1};
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
}

void Renderer::deleteDeferredQuad() {
  glDeleteBuffers(1, &quadVBO);
  glDeleteVertexArrays(1, &quadVAO);
}

void Renderer::gbufferPass(std::shared_ptr<Scene> scene) {
  gbufferPass(scene, g_fbo);
}

void Renderer::gbufferPass(std::shared_ptr<Scene> scene, GLuint fbo) {
  static const auto envCube = NewEnvironmentCube();

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 viewMat = scene->getMainCamera()->getViewMat();
  glm::mat4 projMat = scene->getMainCamera()->getProjectionMat();

  // TODO: debug environment map
  if (auto envmap = scene->getEnvironmentMap()) {
    glDepthMask(GL_FALSE);
    skyboxShader->use();
    skyboxShader->setMatrix("gbufferViewMatrix", viewMat);
    skyboxShader->setMatrix("gbufferProjectionMatrix", projMat);
    skyboxShader->setTexture("skybox", envmap->getId(), 0);
    envCube->getMesh()->draw();
    glDepthMask(GL_TRUE);
  }

  gbufferShader->use();
  gbufferShader->setMatrix("gbufferViewMatrix", viewMat);
  gbufferShader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
  gbufferShader->setMatrix("gbufferProjectionMatrix", projMat);
  gbufferShader->setMatrix("gbufferProjectionMatrixInverse",
                           glm::inverse(projMat));

  for (const auto &obj : scene->getObjects()) {
    auto mesh = obj->getMesh();
    if (!mesh) {
      continue;
    }

    auto shader = obj->shader;
    if (shader) {
      shader->use();
      shader->setMatrix("gbufferViewMatrix", viewMat);
      shader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
      shader->setMatrix("gbufferProjectionMatrix", projMat);
      shader->setMatrix("gbufferProjectionMatrixInverse",
                        glm::inverse(projMat));
    } else {
      shader = gbufferShader;
      shader->use();
    }

    shader->setMatrix("gbufferModelMatrix", obj->getModelMat());
    shader->setVec3("material.ka", obj->material.ka);
    shader->setVec3("material.kd", obj->material.kd);
    shader->setVec3("material.ks", obj->material.ks);
    shader->setFloat("material.ke", obj->material.exp);
    shader->setTexture("material.kd_map", obj->material.kd_map->getId(), 0);
    shader->setBool("material.has_kd_map", obj->material.kd_map->getId() != 0);
    shader->setTexture("material.ks_map", obj->material.ks_map->getId(), 1);
    shader->setBool("material.has_ks_map", obj->material.ks_map->getId() != 0);
    shader->setTexture("material.height_map", obj->material.height_map->getId(),
                       2);
    shader->setBool("material.has_height_map",
                    obj->material.height_map->getId() != 0);
    shader->setTexture("material.normal_map", obj->material.normal_map->getId(),
                       3);
    shader->setBool("material.has_normal_map",
                    obj->material.normal_map->getId() != 0);

    mesh->draw();
  }
}

void Renderer::deferredPass(std::shared_ptr<Scene> scene) {
  deferredPass(scene, 0);
}

void Renderer::deferredPass(std::shared_ptr<Scene> scene, GLuint fbo) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);

  auto mainCam = scene->getMainCamera();

  deferredShader->use();
  glm::mat4 viewMat = mainCam->getViewMat();
  glm::mat4 projMat = mainCam->getProjectionMat();
  deferredShader->setMatrix("gbufferViewMatrix", viewMat);
  deferredShader->setMatrix("gbufferViewMatrixInverse", glm::inverse(viewMat));
  deferredShader->setMatrix("gbufferProjectionMatrix", projMat);
  deferredShader->setMatrix("gbufferProjectionMatrixInverse",
                            glm::inverse(projMat));
  deferredShader->setVec3("cameraPosition", mainCam->position);

  // debug code
  deferredShader->setInt("debug", debug);

  const auto &pointLights = scene->getPointLights();
  for (size_t i = 0; i < pointLights.size(); i++) {
    std::string p = "pointLights[" + std::to_string(i) + "].position";
    std::string e = "pointLights[" + std::to_string(i) + "].emission";
    deferredShader->setVec3(p, pointLights[i].position);
    deferredShader->setVec3(e, pointLights[i].emission);
  }

  const auto &directionalLights = scene->getDirectionalLights();
  for (size_t i = 0; i < directionalLights.size(); i++) {
    std::string d = "directionalLights[" + std::to_string(i) + "].direction";
    std::string e = "directionalLights[" + std::to_string(i) + "].emission";
    deferredShader->setVec3(d, directionalLights[i].direction);
    deferredShader->setVec3(e, directionalLights[i].emission);
  }

  deferredShader->setFloat("near", mainCam->near);
  deferredShader->setFloat("far", mainCam->far);

  for (int n = 0; n < N_COLOR_ATTACHMENTS; n++) {
    deferredShader->setTexture("colortex" + std::to_string(n), colortex[n], n);
  }
  deferredShader->setTexture("depthtex0", depthtex, N_COLOR_ATTACHMENTS);

  // render quad
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Renderer::renderScene(std::shared_ptr<Scene> scene) {
  if (!initialized) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }
  if (!scene->getMainCamera()) {
    fprintf(stderr, "Scene does not have a main camera\n");
    return;
  }
  if (!gbufferShader || !deferredShader) {
    fprintf(stderr, "Shaders not initialized\n");
    return;
  }

  gbufferPass(scene);
  deferredPass(scene);
}

void Renderer::renderSceneToFile(std::shared_ptr<Scene> scene,
                                 std::string filename) {
  if (!initialized) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }
  if (!scene->getMainCamera()) {
    fprintf(stderr, "Scene does not have a main camera\n");
    return;
  }
  if (!gbufferShader || !deferredShader) {
    fprintf(stderr, "Shaders not initialized\n");
    return;
  }

  gbufferPass(scene);
  deferredPass(scene, composite_fbo);

  writeToFile(compositeTex, width, height, filename);
}
} // namespace Optifuser
