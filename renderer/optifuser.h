#pragma once
#include "input.h"
#include "renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>
#include "scene.h"
#include <iostream>

namespace Optifuser {

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
public:
  OffScreenRenderContext(int w, int h) {
    width = w;
    height = h;

    GLuint fbo;
    GLuint tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &tex);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           tex, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "error: Cannot create complete framebuffer");
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

  virtual void render(std::shared_ptr<Scene> scene) override {}
};

class GLFWRenderContext : public RenderContext {
  GLFWwindow *window;
 public:
  Optifuser::Renderer renderer;

public:
  GLFWRenderContext(int w = 480, int h = 640) : renderer(w, h) {
    width = w;
    height = h;
    fbo = 0;
    tex = 0;
    init(width, height);
    renderer.init();
  }

  ~GLFWRenderContext() {
    renderer.exit();
    if (window) {
      glfwDestroyWindow(window);
    }
  }

  GLFWRenderContext(const GLFWRenderContext &) = delete;
  const GLFWRenderContext &operator=(const GLFWRenderContext &) = delete;

  void init(uint32_t width, uint32_t height) {
    if (!glfwInit()) {
      fprintf(stderr, "error: Could not initialize GLFW\n");
      exit(1);
    }
    window = glfwCreateWindow(width, height, "opengl", NULL, NULL);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    const GLubyte *glrenderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    fprintf(stdout, "Renderer: %s\n", glrenderer);
    fprintf(stdout, "OpenGL Version: %s\n", version);
  }

  void display() const { glfwSwapBuffers(window); }
  virtual void render(std::shared_ptr<Scene> scene) override {
    renderer.renderScene(scene);
    display();
  }
};

} // namespace Optifuser
