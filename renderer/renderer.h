#pragma once
#include "scene.h"
#include "shader.h"
#include <GL/glew.h>
#include <map>
#include <stdint.h>

#define N_COLOR_ATTACHMENTS 3
namespace Optifuser {
class Renderer {
public:
  int debug = 0;

public:
  Renderer(GLuint w, GLuint h);
  void init();
  void exit();
  void resize(GLuint w, GLuint h);

private:
  bool initialized;

private:
  std::string gbufferVertShaderFile;
  std::string gbufferFragShaderFile;
  std::shared_ptr<Shader> gbufferShader;

  std::string deferredVertShaderFile;
  std::string deferredFragShaderFile;
  std::shared_ptr<Shader> deferredShader;

  std::string skyboxVertShaderFile;
  std::string skyboxFragShaderFile;
  std::shared_ptr<Shader> skyboxShader;

public:
  void setGBufferShader(const std::string &vs, const std::string &fs);
  void setDeferredShader(const std::string &vs, const std::string &fs);
  void setSkyboxShader(const std::string &vs, const std::string &fs);

protected:
  GLuint width, height;

public:
  inline GLuint getWidth() const { return width; }
  inline GLuint getHeight() const { return height; }

private:
  GLuint g_fbo;
  GLuint colortex[N_COLOR_ATTACHMENTS];
  GLuint depthtex;

  void initGbufferFramebuffer();
  void initColortex();
  void initDepthtex();
  void bindAttachments();

  void deleteGbufferFramebuffer();
  void deleteColortex();
  void deleteDepthtex();

private:
  GLuint quadVAO, quadVBO;
  void initDeferredQuad();
  void deleteDeferredQuad();

private:
  GLuint composite_fbo;
  GLuint compositeTex;
  void initCompositeFramebuffer();
  void initCompositeTex();
  void deleteCompositeFramebuffer();
  void deleteCompositeTex();

private:
  void gbufferPass(std::shared_ptr<Scene> scene);
  void gbufferPass(std::shared_ptr<Scene> scene, GLuint fbo);
  void deferredPass(std::shared_ptr<Scene> scene);
  void deferredPass(std::shared_ptr<Scene> scene, GLuint fbo);

public:
  void renderScene(std::shared_ptr<Scene> scene);
  void renderSceneToFile(std::shared_ptr<Scene> scene, std::string filename);
  void reloadShaders();
};

} // namespace Optifuser
