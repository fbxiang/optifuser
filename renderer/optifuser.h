#pragma once
#include "input.h"
#include "renderer.h"
#include "scene.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>

namespace Optifuser {

bool glfwInitialized = false;
GLFWwindow *mainWindow;

Input input;
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  if (key == GLFW_KEY_Q) {
    glfwSetWindowShouldClose(window, 1);
  }
  input.keyCallback(key, scancode, action, mods);
}

void ensureGlobalContext() {
  if (glfwInitialized) {
    return;
  }
  if (!glfwInit()) {
    fprintf(stderr, "error: Could not initialize GLFW\n");
    exit(1);
  }
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  mainWindow = glfwCreateWindow(1, 1, "opengl", NULL, NULL);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glfwMakeContextCurrent(mainWindow);

  glfwSetKeyCallback(mainWindow, keyCallback);

  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte *glrenderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  fprintf(stdout, "Renderer: %s\n", glrenderer);
  fprintf(stdout, "OpenGL Version: %s\n", version);

  glfwInitialized = true;
}

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
  OffScreenRenderContext(int w, int h) : renderer(w, h) {
    ensureGlobalContext();
    width = w;
    height = h;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_BGRA,
                 GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           tex, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "error: Cannot create complete framebuffer");
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderer.init();
  }

  ~OffScreenRenderContext() {
    if (fbo) {
      glDeleteFramebuffers(1, &fbo);
      glDeleteTextures(1, &tex);
    }
  }

  OffScreenRenderContext(const OffScreenRenderContext &) = delete;
  const OffScreenRenderContext &
  operator=(const OffScreenRenderContext &) = delete;

  virtual void render(std::shared_ptr<Scene> scene) override {
    if (fbo && tex) {
      renderer.renderScene(scene, fbo);
    }
  }

  void save(const std::string &filename) {
    if (fbo && tex) {
      writeToFile(tex, width, height, filename);
    }
  }
};

class GLFWRenderContext : public RenderContext {

public:
  Optifuser::Renderer renderer;

  static GLFWRenderContext &Get(int w = 640, int h = 480) {
    static GLFWRenderContext Instance;
    Instance.init(w, h);
    Instance.renderer.resize(w, h);
    return Instance;
  }

protected:
  GLFWRenderContext(int w = 640, int h = 480) : renderer(w, h) {
    ensureGlobalContext();
    width = w;
    height = h;
    fbo = 0;
    tex = 0;
    init(width, height);
    renderer.init();
  }

  ~GLFWRenderContext() { renderer.exit(); }

public:
  GLFWRenderContext(const GLFWRenderContext &) = delete;
  const GLFWRenderContext &operator=(const GLFWRenderContext &) = delete;

  void init(uint32_t width, uint32_t height) {
    glfwSetWindowSize(mainWindow, width, height);
    glViewport(0, 0, width, height);
    glfwShowWindow(mainWindow);
  }

  void processEvents() {
    glfwPollEvents();
    double xpos, ypos;
    glfwGetCursorPos(mainWindow, &xpos, &ypos);
    input.cursorPosCallback(xpos, ypos);
    input.mouseCallback(
        GLFW_MOUSE_BUTTON_RIGHT,
        glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_RIGHT));
  }

  bool shouldExit() { return glfwWindowShouldClose(mainWindow); }

  virtual void render(std::shared_ptr<Scene> scene) override {
    renderer.renderScene(scene);
    glfwSwapBuffers(mainWindow);
  }
};

} // namespace Optifuser
