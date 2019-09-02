#include "renderer.h"
#include "debug.h"
#include <iostream>
namespace Optifuser {

Renderer::Renderer() {
  for (int n = 0; n < N_COLORTEX; ++n) {
    colortex[n] = 0;
  }
  segtex[0] = segtex[1] = 0;
  for (int n = 0; n < N_FBO; ++n) {
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
}

void Renderer::initTextures() {
  deleteTextures();

  // colortex
  glGenTextures(N_COLORTEX, colortex);
  for (int n = 0; n < N_COLORTEX; n++) {
    glBindTexture(GL_TEXTURE_2D, colortex[n]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA,
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LABEL_TEXTURE(segtex[1], "segmentation color tex");
  }
  // outputtex
  glGenTextures(1, &outputtex);
  glBindTexture(GL_TEXTURE_2D, outputtex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA,
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_width, m_height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  LABEL_TEXTURE(depthtex, "gbuffer depth");
}

void Renderer::renderSegmentation(bool enabled) {
  m_renderSegmentation = enabled;
  initTextures();
  rebindTextures();
}

void Renderer::setGBufferShader(const std::string &vs, const std::string &fs) {
  gbuffer_pass.setShader(vs, fs);
}
void Renderer::setDeferredShader(const std::string &vs, const std::string &fs) {
  lighting_pass.setShader(vs, fs);
}

void Renderer::init() {
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glGenFramebuffers(N_FBO, m_fbo);
  gbuffer_pass.init();
  lighting_pass.init();

  gbuffer_pass.setFbo(m_fbo[0]);
  lighting_pass.setFbo(m_fbo[1]);
  initialized = true;
}

void Renderer::exit() {
  initialized = false;
  deleteTextures();
  glDeleteFramebuffers(N_FBO, m_fbo);
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

  gbuffer_pass.setColorAttachments(n_tex, tex, m_width, m_height);
  gbuffer_pass.setDepthAttachment(depthtex);
  gbuffer_pass.bindAttachments();

  lighting_pass.setAttachment(outputtex, m_width, m_height);
  lighting_pass.setInputTextures(N_COLORTEX, colortex, depthtex);
}

void Renderer::resize(GLuint w, GLuint h) {
  m_width = w;
  m_height = h;
  initTextures();
  rebindTextures();
}

void Renderer::reloadShaders() { std::cerr << "Not implemented" << std::endl; }

// void Renderer::renderAxes(const glm::mat4 &modelMat) {
//   static std::unique_ptr<Object> axes = nullptr;
//   if (!axes) {
//     auto x = NewCube();
//     x->scale = {1, 0.05, 0.05};
//     x->position = {1, 0, 0};
//     x->material.kd = {1, 0, 0};

//     auto y = NewCube();
//     y->scale = {0.05, 1, 0.05};
//     y->position = {0, 1, 0};
//     y->material.kd = {0, 1, 0};

//     auto z = NewCube();
//     z->scale = {0.05, 0.05, 1};
//     z->position = {0, 0, 1};
//     z->material.kd = {0, 0, 1};

//     axes = NewObject<Object>();
//     axes->addChild(std::move(x));
//     axes->addChild(std::move(y));
//     axes->addChild(std::move(z));
//   }

//   glm::mat4 scaling;
//   scaling[0] = glm::vec4(0.1, 0, 0, 0);
//   scaling[1] = glm::vec4(0, 0.1, 0, 0);
//   scaling[2] = glm::vec4(0, 0, 0.1, 0);
//   scaling[3] = glm::vec4(0, 0, 0, 1);

//   renderObjectTree(axes, modelMat * scaling);
// }

void Renderer::renderScene(const Scene &scene, const CameraSpec &camera) {
  if (!initialized) {
    fprintf(stderr, "Renderer is not initialized\n");
    return;
  }

  gbuffer_pass.render(scene, camera, m_renderSegmentation);
  lighting_pass.render(scene, camera);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displayLighting(GLuint fbo) const {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[1]);
  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::displaySegmentation(GLuint fbo) const {
  // read from segmentation color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[2]);
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

void Renderer::displayDepth(GLuint fbo) const {
  // bind depth to color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[2]);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, depthtex, 0);

  // draw to given fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
