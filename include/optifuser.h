#pragma once
#include "input.h"
#include "renderer.h"
#include "scene.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>

namespace Optifuser {

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void ensureGlobalContext();
Input& getInput();

class RenderContext {
protected:
  int width;
  int height;
  GLuint fbo;
  GLuint tex;

public:
  inline int getWidth() { return width; }
  inline int getHeight() { return height; }
  inline GLuint getFbo() { return fbo; }
  inline GLuint getTex() { return tex; }
  virtual void render(std::shared_ptr<Scene> scene) = 0;
};

class OffScreenRenderContext : public RenderContext {
  GLuint fbo;
  GLuint tex;

public:
  Optifuser::Renderer renderer;

public:
  OffScreenRenderContext(int w, int h);

  ~OffScreenRenderContext();

  OffScreenRenderContext(const OffScreenRenderContext &) = delete;
  const OffScreenRenderContext &
  operator=(const OffScreenRenderContext &) = delete;

  virtual void render(std::shared_ptr<Scene> scene) override;

  void save(const std::string &filename);
};

class GLFWRenderContext : public RenderContext {

public:
  Optifuser::Renderer renderer;

  static GLFWRenderContext &Get(int w = 640, int h = 480); 

protected:
  GLFWRenderContext(int w = 640, int h = 480);

  ~GLFWRenderContext();

public:
  GLFWRenderContext(const GLFWRenderContext &) = delete;
  const GLFWRenderContext &operator=(const GLFWRenderContext &) = delete;

  void init(uint32_t width, uint32_t height); 
  void processEvents();
  virtual void render(std::shared_ptr<Scene> scene) override;
};

} // namespace Optifuser
