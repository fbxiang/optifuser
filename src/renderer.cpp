#include "renderer.h"
#include "debug.h"
#include <iostream>
namespace Optifuser {

Renderer::Renderer() {
  for (int n = 0; n < N_COLORTEX; ++n) {
    colortex[n] = 0;
  }
  segtex[0] = segtex[1] = segtex[2] = 0;
  usertex[0] = 0;
  for (int n = 0; n < FBO_TYPE::COUNT; ++n) {
    m_fbo[n] = 0;
  }
}

void Renderer::deleteTextures() {
  glDeleteTextures(1, &depthtex);
  depthtex = 0;

  glDeleteTextures(N_COLORTEX, colortex);
  for (int n = 0; n < N_COLORTEX; ++n) {
    colortex[n] = 0;
  }

  glDeleteTextures(1, &lightingtex);
  lightingtex = 0;

  glDeleteTextures(1, &outputtex);
  outputtex = 0;

  glDeleteTextures(3, segtex);
  segtex[0] = segtex[1] = segtex[2] = 0;

  glDeleteTextures(1, usertex);
  usertex[0] = 0;

  glDeleteTextures(1, &shadowtex);
}

void Renderer::enableAxisPass(bool enable) { axisPassEnabled = enable; }
void Renderer::enableDisplayPass(bool enable) { displayPassEnabled = enable; }

void Renderer::enableGlobalAxes(bool enable) {
  axis_pass.globalAxes = enable;
}

void Renderer::initTextures() {
  deleteTextures();

  // colortex
  glGenTextures(N_COLORTEX, colortex);
  for (int n = 0; n < N_COLORTEX; n++) {
    glBindTexture(GL_TEXTURE_2D, colortex[n]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(colortex[n], "colortex" + std::to_string(n));
  }

  glGenTextures(3, segtex);
  glBindTexture(GL_TEXTURE_2D, segtex[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_width, m_height, 0, GL_RED_INTEGER, GL_INT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(segtex[0], "segmentation tex");

  glBindTexture(GL_TEXTURE_2D, segtex[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_width, m_height, 0, GL_RED_INTEGER, GL_INT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(segtex[1], "segmentation tex 2");

  glBindTexture(GL_TEXTURE_2D, segtex[2]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(segtex[2], "segmentation color tex");

  glGenTextures(1, usertex);
  glBindTexture(GL_TEXTURE_2D, usertex[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(usertex[0], "User Texture 0");

  // lightingtex
  glGenTextures(1, &lightingtex);
  glBindTexture(GL_TEXTURE_2D, lightingtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(lightingtex, "lighting");

  // displaytex 
  glGenTextures(1, &outputtex);
  glBindTexture(GL_TEXTURE_2D, outputtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(outputtex, "output");

  // depthtex
  glGenTextures(1, &depthtex);
  glBindTexture(GL_TEXTURE_2D, depthtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT,
               GL_FLOAT, 0);
  LABEL_TEXTURE(depthtex, "gbuffer depth");

  // GLfloat color[4] = {1.f, 1.f, 1.f, 1.f};
  // glTextureParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

  // shadowtex
  glGenTextures(1, &shadowtex);
  glBindTexture(GL_TEXTURE_2D, shadowtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowWidth, shadowHeight, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  LABEL_TEXTURE(shadowtex, "shadow map");
}

void Renderer::setAxisShader(const std::string &vs, const std::string &fs) {
  axis_pass.setShader(vs, fs);
}

void Renderer::setGBufferShader(const std::string &vs, const std::string &fs) {
  gbuffer_pass.setShader(vs, fs);
}
void Renderer::setDeferredShader(const std::string &vs, const std::string &fs) {
  lighting_pass.setShader(vs, fs);
}
void Renderer::setShadowShader(const std::string &vs, const std::string &fs) {
  shadow_pass.setShader(vs, fs);
}
void Renderer::setTransparencyShader(const std::string &vs, const std::string &fs) {
  transparency_pass.setShader(vs, fs);
}
void Renderer::setDisplayShader(const std::string &vs, const std::string &fs) {
  display_pass.setShader(vs, fs);
}

void Renderer::init(float s) {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glGenFramebuffers(FBO_TYPE::COUNT, m_fbo);
  gbuffer_pass.init();
  lighting_pass.init();
  shadow_pass.init();
  axis_pass.init();
  transparency_pass.init();
  display_pass.init();

  gbuffer_pass.setFbo(m_fbo[FBO_TYPE::GBUFFER]);
  lighting_pass.setFbo(m_fbo[FBO_TYPE::LIGHTING]);
  shadow_pass.setFbo(m_fbo[FBO_TYPE::SHADOW]);
  axis_pass.setFbo(m_fbo[FBO_TYPE::AXIS]);
  transparency_pass.setFbo(m_fbo[FBO_TYPE::TRANSPARENCY]);
  display_pass.setFbo(m_fbo[FBO_TYPE::DISPLAY]);
  initialized = true;

#ifdef _USE_MACOSX
  scaling = s;
#endif
}

void Renderer::exit() {
  initialized = false;
  deleteTextures();
  glDeleteFramebuffers(FBO_TYPE::COUNT, m_fbo);
}

void Renderer::rebindTextures() {
  GLuint tex[N_COLORTEX + 4 + 1];
  int n_tex = N_COLORTEX;
  for (int n = 0; n < N_COLORTEX; ++n) {
    tex[n] = colortex[n];
  }
  tex[N_COLORTEX] = segtex[0];
  tex[N_COLORTEX + 1] = segtex[1];
  tex[N_COLORTEX + 2] = segtex[2];
  tex[N_COLORTEX + 3] = usertex[0];
  n_tex = N_COLORTEX + 4;
  shadow_pass.setDepthAttachment(shadowtex, shadowWidth, shadowHeight);

  gbuffer_pass.setColorAttachments(n_tex, tex, m_width, m_height);
  gbuffer_pass.setDepthAttachment(depthtex);
  gbuffer_pass.bindAttachments();

  lighting_pass.setAttachment(lightingtex, m_width, m_height);
  lighting_pass.setInputTextures(N_COLORTEX, colortex, depthtex);

  tex[N_COLORTEX + 4] = lightingtex;
  transparency_pass.setColorAttachments(n_tex + 1, tex, m_width, m_height);
  transparency_pass.setDepthAttachment(depthtex);
  transparency_pass.bindAttachments();

  axis_pass.setColorAttachments(1, &lightingtex, m_width, m_height);
  axis_pass.setDepthAttachment(depthtex);
  axis_pass.bindAttachments();

  // display_pass.setInputTextures(n_tex + 1, tex, depthtex);
  display_pass.setAttachment(outputtex, m_width, m_height);
  display_pass.setInputTextures(N_COLORTEX, colortex, depthtex);
}

void Renderer::resize(GLuint w, GLuint h) {
  m_width = w * scaling;
  m_height = h * scaling;

  initTextures();
  rebindTextures();
}

void Renderer::reloadShaders() { std::cerr << "Not implemented" << std::endl; }

void Renderer::renderScene(Scene &scene, const CameraSpec &camera) {
  if (!initialized) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }
  auto &lights = scene.getDirectionalLights();
  scene.prepareObjects();
  if (lights.size()) {
    shadow_pass.render(scene, camera);
    lighting_pass.setShadowTexture(shadowtex);
  }
  gbuffer_pass.render(scene, camera, true);
  lighting_pass.render(scene, camera);
  if (axisPassEnabled) {
    axis_pass.render(scene, camera);
  }
  transparency_pass.render(scene, camera, true);

  if (displayPassEnabled) {
    display_pass.render();
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void Renderer::displayLighting(GLuint fbo) const {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightingtex, 0);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displayUserTexture(GLuint fbo) const {
  // read from segmentation color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, usertex[0], 0);

  // draw to given fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displaySegmentation(GLuint fbo) const {
  // read from segmentation color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, segtex[2], 0);

  // draw to given fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::display(GLuint fbo) const {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputtex, 0);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::setObjectIdForAxis(int id) { axis_pass.setObjectId(id); }

void Renderer::saveLighting(const std::string &file, bool raw) {
  if (raw) {
    writeTextureRGBAFloat32Raw(lightingtex, m_width, m_height, file);
  } else {
    std::cerr << "Saving texture without raw is not supported" << std::endl;
  }
}

void Renderer::saveNormal(const std::string &file, bool raw) {
  if (raw) {
    writeTextureRGBAFloat32Raw(colortex[2], m_width, m_height, file);
  } else {
    std::cerr << "Saving texture without raw is not supported" << std::endl;
  }
}

void Renderer::saveDepth(const std::string &file, bool raw) {
  if (raw) {
    writeTextureDepthFloat32Raw(depthtex, m_width, m_height, file);
  } else {
    std::cerr << "Saving texture without raw is not supported" << std::endl;
  }
}

std::vector<float> Renderer::getLighting() {
  return getRGBAFloat32Texture(lightingtex, m_width, m_height);
}
std::vector<float> Renderer::getAlbedo() {
  return getRGBAFloat32Texture(colortex[0], m_width, m_height);
}
std::vector<float> Renderer::getNormal() {
  return getRGBAFloat32Texture(colortex[2], m_width, m_height);
}
std::vector<float> Renderer::getDepth() {
  return getDepthFloat32Texture(depthtex, m_width, m_height);
}
std::vector<int> Renderer::getSegmentation() {
  return getInt32Texture(segtex[0], m_width, m_height);
}
std::vector<int> Renderer::getSegmentation2() {
  return getInt32Texture(segtex[1], m_width, m_height);
}
std::vector<float> Renderer::getUserTexture() {
  return getRGBAFloat32Texture(usertex[0], m_width, m_height);
}

void Renderer::enablePicking() { glGenFramebuffers(1, &pickingFbo); }

int Renderer::pickSegmentationId(int x, int y) {
  if (!pickingFbo) {
    std::cerr << "failed to pick segmentation id, you need to enable picking first." << std::endl;
    return 0;
  }
  // only valid when using segmentation
  if (x < 0 || x >= static_cast<int>(m_width) || y < 0 || y >= static_cast<int>(m_height)) {
    return 0;
  }
  int value;
  glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, segtex[0], 0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  // pixel position is upside down
  glReadPixels(x, m_height - y, 1, 1, GL_RED_INTEGER, GL_INT, &value);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return value;
}

int Renderer::pickObjectId(int x, int y) {
  if (!pickingFbo) {
    std::cerr << "failed to pick object id, you need to enable picking first." << std::endl;
    return 0;
  }
  // only valid when using segmentation
  if (x < 0 || x >= static_cast<int>(m_width) || y < 0 || y >= static_cast<int>(m_height)) {
    return 0;
  }
  int value;
  glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, segtex[1], 0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  // pixel position is upside down
  glReadPixels(x, m_height - y, 1, 1, GL_RED_INTEGER, GL_INT, &value);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return value;
}

} // namespace Optifuser
