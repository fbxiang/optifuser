#pragma once
#include "input.h"
#include "optix_renderer.h"
#include "renderer.h"
#include "scene.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>

namespace Optifuser {

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);
void ensureGlobalContext();
Input &getInput();

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
};

class GLFWRenderContext : public RenderContext {

public:
  Optifuser::Renderer renderer;

  static GLFWRenderContext &Get(int w = 640, int h = 480);

  void initGui();

protected:
  GLFWRenderContext(int w = 640, int h = 480);

  ~GLFWRenderContext();

public:
  GLFWRenderContext(const GLFWRenderContext &) = delete;
  const GLFWRenderContext &operator=(const GLFWRenderContext &) = delete;

  void init(uint32_t width, uint32_t height);
  void processEvents();
  void swapBuffers() const;
  GLFWwindow *getWindow() const;

  void showWindow();
  void hideWindow();

  void destroy();
};

class OffscreenRenderContext : public RenderContext {
public:
  Optifuser::Renderer renderer;
  inline static std::unique_ptr<OffscreenRenderContext> Create(int w, int h) {
    return std::make_unique<OffscreenRenderContext>(w, h);
  }

  ~OffscreenRenderContext();
  OffscreenRenderContext(int w, int h);
};

class OptixContext : public RenderContext {
public:
  Optifuser::OptixRenderer renderer;
  inline static std::unique_ptr<OptixContext> Create(int w, int h) {
    return std::make_unique<OptixContext>(w, h);
  }

  OptixContext(int w, int h);
};

} // namespace Optifuser
