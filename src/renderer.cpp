#include "renderer.h"
#include "debug.h"
#include <iostream>
namespace Optifuser {

Renderer::Renderer() {
  for (int n = 0; n < N_COLORTEX; ++n) {
    colortex[n] = 0;
  }
  segtex[0] = segtex[1] = 0;
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

  glDeleteTextures(1, &outputtex);
  outputtex = 0;

  glDeleteTextures(2, segtex);
  segtex[0] = segtex[1] = 0;

  glDeleteTextures(1, &shadowtex);
}

void Renderer::initTextures() {
  deleteTextures();

  // colortex
  glGenTextures(N_COLORTEX, colortex);
  for (int n = 0; n < N_COLORTEX; n++) {
    glBindTexture(GL_TEXTURE_2D, colortex[n]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(colortex[n], "colortex" + std::to_string(n));
  }

  if (m_renderSegmentation) {
    glGenTextures(2, segtex);
    glBindTexture(GL_TEXTURE_2D, segtex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_width, m_height, 0,
                 GL_RED_INTEGER, GL_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(segtex[0], "segmentation tex");

    glBindTexture(GL_TEXTURE_2D, segtex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(segtex[1], "segmentation color tex");
  }
  // outputtex
  glGenTextures(1, &outputtex);
  glBindTexture(GL_TEXTURE_2D, outputtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  LABEL_TEXTURE(outputtex, "output");

  // depthtex
  glGenTextures(1, &depthtex);
  glBindTexture(GL_TEXTURE_2D, depthtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTextureParameterf(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  LABEL_TEXTURE(depthtex, "gbuffer depth");

  // shadowtex
  glGenTextures(1, &shadowtex);
  glBindTexture(GL_TEXTURE_2D, shadowtex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowWidth,
               shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  LABEL_TEXTURE(shadowtex, "shadow map");
}

void Renderer::renderSegmentation(bool enabled) {
  m_renderSegmentation = enabled;
  initTextures();
  rebindTextures();
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

void Renderer::init() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glGenFramebuffers(FBO_TYPE::COUNT, m_fbo);
  gbuffer_pass.init();
  lighting_pass.init();
  shadow_pass.init();
  axis_pass.init();

  gbuffer_pass.setFbo(m_fbo[FBO_TYPE::GBUFFER]);
  lighting_pass.setFbo(m_fbo[FBO_TYPE::LIGHTING]);
  shadow_pass.setFbo(m_fbo[FBO_TYPE::SHADOW]);
  axis_pass.setFbo(m_fbo[FBO_TYPE::AXIS]);
  initialized = true;
}

void Renderer::exit() {
  initialized = false;
  deleteTextures();
  glDeleteFramebuffers(FBO_TYPE::COUNT, m_fbo);
}

void Renderer::rebindTextures() {
  GLuint tex[N_COLORTEX + 2];
  int n_tex = N_COLORTEX;
  for (int n = 0; n < N_COLORTEX; ++n) {
    tex[n] = colortex[n];
  }
  if (m_renderSegmentation) {
    tex[N_COLORTEX] = segtex[0];
    tex[N_COLORTEX + 1] = segtex[1];
    n_tex = N_COLORTEX + 2;
  }
  shadow_pass.setDepthAttachment(shadowtex, shadowWidth, shadowHeight);

  gbuffer_pass.setColorAttachments(n_tex, tex, m_width, m_height);
  gbuffer_pass.setDepthAttachment(depthtex);
  gbuffer_pass.bindAttachments();

  lighting_pass.setAttachment(outputtex, m_width, m_height);
  lighting_pass.setInputTextures(N_COLORTEX, colortex, depthtex);

  axis_pass.setColorAttachments(1, &outputtex, m_width, m_height);
  axis_pass.setDepthAttachment(depthtex);
  axis_pass.bindAttachments();
}

void Renderer::resize(GLuint w, GLuint h) {
  m_width = w;
  m_height = h;
  initTextures();
  rebindTextures();
}

void Renderer::reloadShaders() { std::cerr << "Not implemented" << std::endl; }

void Renderer::renderScene(const Scene &scene, const CameraSpec &camera) {
  if (!initialized) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }
  auto &lights = scene.getDirectionalLights();
  if (lights.size()) {
    shadow_pass.render(scene, camera);
    lighting_pass.setShadowTexture(shadowtex);
  }
  gbuffer_pass.render(scene, camera, m_renderSegmentation);
  lighting_pass.render(scene, camera);
  axis_pass.render(scene, camera);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displayLighting(GLuint fbo) const {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::LIGHTING]);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displaySegmentation(GLuint fbo) const {
  // read from segmentation color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[FBO_TYPE::COPY]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, segtex[1], 0);

  // draw to given fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::setObjectIdForAxis(int id) {
  axis_pass.setObjectId(id);
}

void Renderer::saveLighting(const std::string &file, bool raw) {
  if (raw) {
    writeTextureRGBAFloat32Raw(outputtex, m_width, m_height, file);
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

// void Renderer::renderSceneToFile(std::shared_ptr<Scene> scene,
//                                  std::string filename) {
//   if (!initialized) {
//     fprintf(stderr, "Renderer is not initialized\n");
//     return;
//   }
//   if (!scene->getMainCamera()) {
//     fprintf(stderr, "Scene does not have a main camera\n");
//     return;
//   }
//   if (!gbufferShader || !deferredShader) {
//     fprintf(stderr, "Shaders not initialized\n");
//     return;
//   }

//   gbufferPass(scene);
//   deferredPass(scene, composite_fbo);

//   writeToFile(compositeTex, width, height, filename);
// }

} // namespace Optifuser
