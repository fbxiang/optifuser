#include "optifuser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Optifuser {

bool glfwInitialized = false;
GLFWwindow *mainWindow;
Input input;

Input &getInput() { return input; }

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
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

GLFWRenderContext &GLFWRenderContext::Get(int w, int h) {
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
  renderer.init();
  renderer.resize(w, h);
}

GLFWRenderContext::~GLFWRenderContext() { renderer.exit(); }

void GLFWRenderContext::init(uint32_t width, uint32_t height) {
  glfwSetWindowSize(mainWindow, width, height);
  glViewport(0, 0, width, height);
  glfwShowWindow(mainWindow);
}

void GLFWRenderContext::initGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
  ImGui_ImplOpenGL3_Init("#version 450");
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

void GLFWRenderContext::destroy() {
  glfwDestroyWindow(mainWindow);
}

GLFWwindow *GLFWRenderContext::getWindow() const {
  return mainWindow;
}

} // namespace Optifuser
