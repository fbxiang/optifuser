#include "passes/transparency_pass.h"
#include <iostream>

constexpr int COLOR_TABLE_SIZE = 60;
static glm::vec3 colortable[COLOR_TABLE_SIZE] = {
    {0.8, 0.4, 0.4},   {0.8, 0.32, 0.32}, {0.8, 0.24, 0.24}, {0.8, 0.52, 0.4},  {0.8, 0.46, 0.32},
    {0.8, 0.41, 0.24}, {0.8, 0.64, 0.4},  {0.8, 0.61, 0.32}, {0.8, 0.58, 0.24}, {0.8, 0.76, 0.4},
    {0.8, 0.75, 0.32}, {0.8, 0.74, 0.24}, {0.72, 0.8, 0.4},  {0.7, 0.8, 0.32},  {0.69, 0.8, 0.24},
    {0.6, 0.8, 0.4},   {0.56, 0.8, 0.32}, {0.52, 0.8, 0.24}, {0.48, 0.8, 0.4},  {0.42, 0.8, 0.32},
    {0.35, 0.8, 0.24}, {0.4, 0.8, 0.44},  {0.32, 0.8, 0.37}, {0.24, 0.8, 0.3},  {0.4, 0.8, 0.56},
    {0.32, 0.8, 0.51}, {0.24, 0.8, 0.46}, {0.4, 0.8, 0.68},  {0.32, 0.8, 0.66}, {0.24, 0.8, 0.63},
    {0.4, 0.8, 0.8},   {0.32, 0.8, 0.8},  {0.24, 0.8, 0.8},  {0.4, 0.68, 0.8},  {0.32, 0.66, 0.8},
    {0.24, 0.63, 0.8}, {0.4, 0.56, 0.8},  {0.32, 0.51, 0.8}, {0.24, 0.46, 0.8}, {0.4, 0.44, 0.8},
    {0.32, 0.37, 0.8}, {0.24, 0.3, 0.8},  {0.48, 0.4, 0.8},  {0.42, 0.32, 0.8}, {0.35, 0.24, 0.8},
    {0.6, 0.4, 0.8},   {0.56, 0.32, 0.8}, {0.52, 0.24, 0.8}, {0.72, 0.4, 0.8},  {0.7, 0.32, 0.8},
    {0.69, 0.24, 0.8}, {0.8, 0.4, 0.76},  {0.8, 0.32, 0.75}, {0.8, 0.24, 0.74}, {0.8, 0.4, 0.64},
    {0.8, 0.32, 0.61}, {0.8, 0.24, 0.58}, {0.8, 0.4, 0.52},  {0.8, 0.32, 0.46}, {0.8, 0.24, 0.41}};

namespace Optifuser {
TransparencyPass::TransparencyPass()
    : m_fbo(0), m_depthtex(0), m_shadowtex(0), m_width(0), m_height(0), m_initialized(false) {}

void TransparencyPass::init() { m_initialized = true; }

void TransparencyPass::setShader(const std::string &vs, const std::string &fs) {
  m_vertFile = vs;
  m_fragFile = fs;
  m_shader = std::make_shared<Shader>(vs.c_str(), fs.c_str());
  if (!m_shader) {
    std::cerr << "Transparency Pass Shader Creation Failed." << std::endl;
  }
}

void TransparencyPass::setFbo(GLuint fbo) { m_fbo = fbo; }

void TransparencyPass::setShadowTexture(GLuint shadowtex, int size) {
  m_shadowtex = shadowtex;
  m_shadowtex_size = size;
}

void TransparencyPass::setColorAttachments(int num, GLuint *tex, int width, int height) {
  m_width = width;
  m_height = height;
  m_colortex.resize(0);
  m_colortex.insert(m_colortex.begin(), tex, tex + num);
}

void TransparencyPass::setDepthAttachment(GLuint depthtex) { m_depthtex = depthtex; }

static void renderObjectTree(const Object &obj, Shader *shader, bool renderSegmentation) {

  glm::mat4 modelMat = obj.globalModelMatrix;
  auto mesh = obj.getMesh();
  if (renderSegmentation) {
    shader->setInt("segmentation", obj.getSegmentId());
    shader->setInt("segmentation2", obj.getObjId());
    shader->setVec3("segmentation_color", colortable[obj.getSegmentId() % COLOR_TABLE_SIZE]);
  }

  shader->setMatrix("gbufferModelMatrix", modelMat);
  shader->setMatrix("gbufferModelMatrixInverse", glm::inverse(modelMat));
  // shader->setVec3("material.ka", obj.material.ka);
  shader->setVec4("material.kd", obj.pbrMaterial->kd);
  shader->setFloat("material.ks", obj.pbrMaterial->ks);
  shader->setFloat("material.roughness", obj.pbrMaterial->roughness);
  shader->setFloat("material.metallic", obj.pbrMaterial->metallic);
  shader->setTexture("material.kd_map", obj.pbrMaterial->kd_map->getId(), 0);
  shader->setBool("material.has_kd_map", obj.pbrMaterial->kd_map->getId() != 0);
  shader->setTexture("material.ks_map", obj.pbrMaterial->ks_map->getId(), 1);
  shader->setBool("material.has_ks_map", obj.pbrMaterial->ks_map->getId() != 0);
  shader->setTexture("material.height_map", obj.pbrMaterial->height_map->getId(), 2);
  shader->setBool("material.has_height_map", obj.pbrMaterial->height_map->getId() != 0);
  shader->setTexture("material.normal_map", obj.pbrMaterial->normal_map->getId(), 3);
  shader->setBool("material.has_normal_map", obj.pbrMaterial->normal_map->getId() != 0);
  shader->setFloat("opacity", obj.visibility);
  auto &userData = obj.getUserData();
  shader->setUserData("user_data", userData.size(), userData.data());
  mesh->draw();
}

void TransparencyPass::render(const Scene &scene, const CameraSpec &camera,
                              bool renderSegmentation) const {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, m_width, m_height);

  if (m_depthtex) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  }

  glm::mat4 viewMat = camera.getViewMat();
  glm::mat4 viewMatInv = glm::inverse(viewMat);
  glm::mat4 projMat = camera.getProjectionMat();
  glm::mat4 projMatInv = glm::inverse(projMat);

  m_shader->use();
  m_shader->setMatrix("gbufferViewMatrix", viewMat);
  m_shader->setMatrix("gbufferViewMatrixInverse", viewMatInv);
  m_shader->setMatrix("gbufferProjectionMatrix", projMat);
  m_shader->setMatrix("gbufferProjectionMatrixInverse", projMatInv);
  m_shader->setMatrix("environmentViewMatrix", viewMat);
  m_shader->setMatrix("environmentViewMatrixInverse", viewMatInv);
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

  // randomtex
  m_shader->setTexture("randomtex", m_randomtex, 5);
  m_shader->setInt("randomtexWidth", m_randomtex_width);
  m_shader->setInt("randomtexHeight", m_randomtex_height);
  m_shader->setInt("viewWidth", m_width);
  m_shader->setInt("viewHeight", m_height);

  // shadow map
  if (m_shadowtex && directionalLights.size()) {
    glm::vec3 dir = directionalLights[0].direction;

    glm::mat4 w2c = glm::translate(glm::mat4(1), -camera.position);
    glm::vec3 csLightDir = w2c * glm::vec4(dir, 0);
    glm::mat4 c2l = glm::lookAt(glm::vec3(0, 0, 0), csLightDir, glm::vec3(0, 1, 0));

    float v = m_shadow_frustum_size;
    glm::mat4 lsProj = glm::ortho(-v, v, -v, v, -v, camera.far);

    m_shader->setTexture("shadowtex", m_shadowtex, 4);
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

  for (const auto &obj : scene.getTransparentObjects()) {
    renderObjectTree(*obj, m_shader.get(), renderSegmentation);
  }
  glDisable(GL_BLEND);
}

int TransparencyPass::numColorAttachments() const { return m_colortex.size(); }

void TransparencyPass::bindAttachments() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  int count = numColorAttachments();
  GLuint attachments[count];
  for (int n = 0; n < count; ++n) {
    glBindTexture(GL_TEXTURE_2D, m_colortex[n]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, GL_TEXTURE_2D, m_colortex[n],
                           0);
    attachments[n] = GL_COLOR_ATTACHMENT0 + n;
  }
  glDrawBuffers(count, attachments);

  glBindTexture(GL_TEXTURE_2D, m_depthtex);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthtex, 0);
}

void TransparencyPass::setRandomTexture(GLuint randomtex, GLuint width, GLuint height) {
  m_randomtex = randomtex;
  m_randomtex_width = width;
  m_randomtex_height = height;
}

void TransparencyPass::setShadowFrustumSize(int size) { m_shadow_frustum_size = size; }

} // namespace Optifuser
