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

  glDeleteTextures(1, &lightingtex2);
  lightingtex2 = 0;

  glDeleteTextures(1, &outputtex);
  outputtex = 0;

  glDeleteTextures(1, &aotex);
  aotex = 0;

  glDeleteTextures(3, segtex);
  segtex[0] = segtex[1] = segtex[2] = 0;

  glDeleteTextures(1, usertex);
  usertex[0] = 0;

  glDeleteTextures(1, &shadowtex);
}

void Renderer::enableShadowPass(bool enable, int shadowmapSize, float shadowFrustumSize) {
  shadowPassEnabled = enable;
  m_shadowSize = shadowmapSize;
  m_shadowFrustumSize = shadowFrustumSize;
  if (initialized) {
    exit();
    init(scaling);
    initTextures();
    rebindTextures();
  }
}

void Renderer::enableAOPass(bool enable) {
  aoPassEnabled = true;
  if (initialized) {
    exit();
    init(scaling);
    initTextures();
    rebindTextures();
  }
}

void Renderer::enableAxisPass(bool enable) {
  axisPassEnabled = enable;
  if (initialized) {
    exit();
    init(scaling);
    initTextures();
    rebindTextures();
  }
}
void Renderer::enableDisplayPass(bool enable) {
  displayPassEnabled = enable;
  if (initialized) {
    exit();
    init(scaling);
    initTextures();
    rebindTextures();
  }
}

void Renderer::enableGlobalAxes(bool enable) {
  if (axisPassEnabled) {
    axis_pass->globalAxes = enable;
  }
}

void Renderer::initTextures() {
  randomtex = CreateRandomTexture(256, 256, 0);

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

  if (aoPassEnabled) {
    glGenTextures(1, &aotex);
    glBindTexture(GL_TEXTURE_2D, aotex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(aotex, "aotex");
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

  // alternative lighting tex for compositing
  glGenTextures(1, &lightingtex2);
  glBindTexture(GL_TEXTURE_2D, lightingtex2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(lightingtex2, "lighting2");

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

  if (shadowPassEnabled) {
    // shadowtex
    glGenTextures(1, &shadowtex);
    glBindTexture(GL_TEXTURE_2D, shadowtex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_shadowSize, m_shadowSize, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    LABEL_TEXTURE(shadowtex, "shadow map");
  }
}

void Renderer::setAxisShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  if (axisPassEnabled) {
    axis_pass->setShader(vs, fs);
  }
}

void Renderer::setGBufferShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  gbuffer_pass->setShader(vs, fs);
}

void Renderer::setAOShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  if (aoPassEnabled) {
    ao_pass->setShader(vs, fs);
  }
}

void Renderer::setDeferredShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  lighting_pass->setShader(vs, fs);
}

void Renderer::setShadowShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  if (shadowPassEnabled) {
    shadow_pass->setShader(vs, fs);
  }
}

void Renderer::setTransparencyShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  transparency_pass->setShader(vs, fs);
}

void Renderer::setCompositeShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  composite_pass->setShader(vs, fs);
}

void Renderer::setDisplayShader(const std::string &vs, const std::string &fs) {
  if (!initialized) {
    throw std::runtime_error("Initialization required before setting shader");
  }
  if (displayPassEnabled) {
    display_pass->setShader(vs, fs);
  }
}

void Renderer::init(float s) {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glGenFramebuffers(FBO_TYPE::COUNT, m_fbo);

  if (!gbuffer_pass) {
    gbuffer_pass = std::make_unique<GBufferPass>();
    gbuffer_pass->init();
  }
  gbuffer_pass->setFbo(m_fbo[FBO_TYPE::GBUFFER]);

  if (!aoPassEnabled) {
    ao_pass = nullptr;
  } else {
    if (!ao_pass) {
      ao_pass = std::make_unique<AOPass>();
      ao_pass->init();
    }
    ao_pass->setFbo(m_fbo[FBO_TYPE::AO]);
  }

  if (!lighting_pass) {
    lighting_pass = std::make_unique<LightingPass>();
    lighting_pass->init();
  }
  lighting_pass->setFbo(m_fbo[FBO_TYPE::LIGHTING]);

  if (!shadowPassEnabled) {
    shadow_pass = nullptr;
  } else {
    if (!shadow_pass) {
      shadow_pass = std::make_unique<ShadowPass>();
      shadow_pass->init();
    }
    shadow_pass->setFbo(m_fbo[FBO_TYPE::SHADOW]);
    shadow_pass->setFrustumSize(m_shadowFrustumSize);
    lighting_pass->setShadowFrustumSize(m_shadowFrustumSize);
  }

  if (!axisPassEnabled) {
    axis_pass = nullptr;
  } else {
    if (!axis_pass) {
      axis_pass = std::make_unique<AxisPass>();
      axis_pass->init();
    }
    axis_pass->setFbo(m_fbo[FBO_TYPE::AXIS]);
  }

  if (!transparency_pass) {
    transparency_pass = std::make_unique<TransparencyPass>();
    transparency_pass->init();
  }
  transparency_pass->setFbo(m_fbo[FBO_TYPE::TRANSPARENCY]);

  if (!composite_pass) {
    composite_pass = std::make_unique<CompositePass>();
    composite_pass->init();
  }
  composite_pass->setFbo(m_fbo[FBO_TYPE::COMPOSITE]);

  if (!displayPassEnabled) {
    display_pass = nullptr;
  } else {
    if (!display_pass) {
      display_pass = std::make_unique<CompositePass>();
      display_pass->init();
    }
    display_pass->setFbo(m_fbo[FBO_TYPE::DISPLAY]);
  }

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
  if (shadowPassEnabled) {
    shadow_pass->setDepthAttachment(shadowtex, m_shadowSize, m_shadowSize);
    lighting_pass->setShadowTexture(shadowtex, m_shadowSize);
  }

  gbuffer_pass->setColorAttachments(n_tex, tex, m_width, m_height);
  gbuffer_pass->setDepthAttachment(depthtex);
  gbuffer_pass->bindAttachments();

  if (aoPassEnabled) {
    ao_pass->setAttachment(aotex, m_width, m_height);
    ao_pass->setInputTextures(N_COLORTEX, colortex, depthtex);
    ao_pass->setRandomTexture(randomtex->getId(), randomtex->getWidth(), randomtex->getHeight());
  }

  lighting_pass->setAttachment(lightingtex, m_width, m_height);
  lighting_pass->setInputTextures(N_COLORTEX, colortex, depthtex);
  lighting_pass->setRandomTexture(randomtex->getId(), randomtex->getWidth(),
                                  randomtex->getHeight());
  lighting_pass->setAOTexture(aotex);

  tex[N_COLORTEX + 4] = lightingtex;
  transparency_pass->setColorAttachments(n_tex + 1, tex, m_width, m_height);
  transparency_pass->setDepthAttachment(depthtex);
  transparency_pass->bindAttachments();

  if (axisPassEnabled) {
    axis_pass->setColorAttachments(1, &lightingtex, m_width, m_height);
    axis_pass->setDepthAttachment(depthtex);
    axis_pass->bindAttachments();
  }

  composite_pass->setAttachment(lightingtex2, m_width, m_height);
  composite_pass->setInputTextures(n_tex + 1, tex, depthtex);
  composite_pass->setRandomTexture(randomtex->getId(), randomtex->getWidth(),
                                   randomtex->getHeight());

  tex[N_COLORTEX + 4] = lightingtex2;

  if (displayPassEnabled) {
    display_pass->setAttachment(outputtex, m_width, m_height);
    display_pass->setInputTextures(n_tex + 1, tex, depthtex);
  }
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
  if (lights.size() && shadowPassEnabled) {
    shadow_pass->render(scene, camera);
  }
  gbuffer_pass->render(scene, camera, true);
  if (aoPassEnabled) {
    ao_pass->render(camera);
  }
  lighting_pass->render(scene, camera);
  if (axisPassEnabled) {
    axis_pass->render(scene, camera);
  }
  transparency_pass->render(scene, camera, true);
  composite_pass->render();

  if (displayPassEnabled) {
    display_pass->render();
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displayLighting(GLuint fbo) const {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightingtex2,
                         0);

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

void Renderer::setObjectIdForAxis(int id) { axis_pass->setObjectId(id); }

std::vector<float> Renderer::getLighting() {
  return getRGBAFloat32Texture(lightingtex2, m_width, m_height);
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
