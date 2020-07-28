#include "optifuser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Optifuser {

bool glfwInitialized = false;
GLFWwindow *mainWindow;
Input input;

Input &getInput() { return input; }

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  input.keyCallback(key, scancode, action, mods);
}

void wheelCallback(GLFWwindow *window, double xoffset, double yoffset) {
  input.wheelCallback(xoffset, yoffset);
}

void ensureGlobalContext() {
  if (glfwInitialized) {
    return;
  }
  if (!glfwInit()) {
    fprintf(stderr, "error: Could not initialize GLFW\n");
    exit(1);
  }
#ifdef _USE_MACOSX
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  mainWindow = glfwCreateWindow(1, 1, "opengl", NULL, NULL);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glfwMakeContextCurrent(mainWindow);

  glfwSetKeyCallback(mainWindow, keyCallback);
  glfwSetScrollCallback(mainWindow, wheelCallback);

  glewExperimental = GL_TRUE;
  glewInit();

#ifdef _VERBOSE
  const GLubyte *glrenderer = glGetString(GL_RENDERER);
  const GLubyte *version = glGetString(GL_VERSION);
  fprintf(stdout, "Renderer: %s\n", glrenderer);
  fprintf(stdout, "OpenGL Version: %s\n", version);
#endif

  glfwInitialized = true;
}

GLFWRenderContext &GLFWRenderContext::Get(int w, int h) {
  if (!spdlog::get("Optifuser")) {
    auto logger =spdlog::stderr_color_mt("Optifuser");
  }
  static GLFWRenderContext Instance;
  Instance.init(w, h);
  return Instance;
}

GLFWRenderContext::GLFWRenderContext(int w, int h) {
  ensureGlobalContext();
  width = w;
  height = h;
  fbo = 0;
  tex = 0;

  init(width, height);

#ifdef _USE_MACOSX
  renderer.init(2);
#else
  renderer.init();
#endif
  renderer.resize(w, h);
}

GLFWRenderContext::~GLFWRenderContext() { renderer.exit(); }

void GLFWRenderContext::init(uint32_t width, uint32_t height) {
  glfwSetWindowSize(mainWindow, width, height);
  glViewport(0, 0, width, height);
}

void GLFWRenderContext::showWindow() { glfwShowWindow(mainWindow); }

void GLFWRenderContext::hideWindow() { glfwHideWindow(mainWindow); }

void GLFWRenderContext::initGui(const std::string &version) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
  ImGui_ImplOpenGL3_Init(("#version " + version).c_str());
}

void GLFWRenderContext::processEvents() {
  glfwPollEvents();
  double xpos, ypos;
  glfwGetCursorPos(mainWindow, &xpos, &ypos);
  input.cursorPosCallback(xpos, ypos);
  input.mouseCallback(GLFW_MOUSE_BUTTON_RIGHT,
                      glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_RIGHT));

  if (!ImGui::GetIO().WantCaptureMouse) {
    input.mouseCallback(GLFW_MOUSE_BUTTON_LEFT,
                        glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_LEFT));
  }

  int newWidth, newHeight;
  glfwGetWindowSize(mainWindow, &newWidth, &newHeight);
  if (width != newWidth || height != newHeight) {
    width = newWidth;
    height = newHeight;
    renderer.resize(width, height);
  }
}

void GLFWRenderContext::swapBuffers() const {
  glfwSwapBuffers(mainWindow);
  getInput().nextFrame();
}

void GLFWRenderContext::destroy() { glfwDestroyWindow(mainWindow); }

GLFWwindow *GLFWRenderContext::getWindow() const { return mainWindow; }

OffscreenRenderContext::OffscreenRenderContext(int w, int h) {
  ensureGlobalContext();
  width = w;
  height = h;
  glGenFramebuffers(1, &fbo);
  renderer.init();
  renderer.resize(w, h);
}

OffscreenRenderContext::~OffscreenRenderContext() {
  renderer.exit();
  glDeleteFramebuffers(1, &fbo);
}

#ifdef _USE_OPTIX
OptixContext::OptixContext(int w, int h, const std::string &ptxDir) : renderer(ptxDir) {
  ensureGlobalContext();
  renderer.init(w, h);
}
#endif

} // namespace Optifuser
