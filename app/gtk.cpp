#include "objectLoader.h"
#include "optifuser.h"
#include "safe_queue.h"
#include <GL/glew.h>
#include <gdkmm-3.0/gdkmm.h>
#include <gtkmm-3.0/gtkmm.h>
#include <iostream>
#include <memory>
#include <thread>
#include <experimental/filesystem>
#include "partnet_loader.hpp"

namespace fs = std::experimental::filesystem;

#define CHECK(exp, msg)                                                        \
  {                                                                            \
    if (!exp) {                                                                \
      std::cerr << msg << std::endl;                                           \
      exit(1);                                                                 \
    }                                                                          \
  }

void loadScene(Optifuser::Scene &scene) {
  loadPartNet("../assets/46627");
  // int id = 0;
  // for (const auto &f : fs::directory_iterator("../assets/46627/textured_objs")) {
  //   if (f.path().extension().u8string() == ".obj") {
  //     auto objs = Optifuser::LoadObj(f.path().u8string(), true, {0, 1, 0}, {0, 0, -1});
  //     ++id;
  //     for (auto & obj : objs) {
  //       obj->setSegmentId(id);
  //       scene.addObject(std::move(obj));
  //     }
  //   }
  // }
}


class MainWindow;
Optifuser::GLFWRenderContext *gContext;
MainWindow *window = nullptr;

SafeQueue<std::string> gRenderQueue;
SafeQueue<std::string> gControlQueue;

class MainWindow : public Gtk::ApplicationWindow {
private:
  Glib::RefPtr<Gtk::Builder> builder;

  Gtk::RadioButton *mLightingButton = nullptr;
  Gtk::RadioButton *mDepthButton = nullptr;
  Gtk::RadioButton *mTextureButton = nullptr;
  Gtk::RadioButton *mSegmentationButton = nullptr;
  Gtk::RadioButton *mWireframeButton = nullptr;

public:
  MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const &builder)
      : Gtk::ApplicationWindow(obj), builder{builder} {
    builder->get_widget("render_mode_lighting", mLightingButton);
    builder->get_widget("render_mode_depth", mDepthButton);
    builder->get_widget("render_mode_texture", mTextureButton);
    builder->get_widget("render_mode_segmentation", mSegmentationButton);
    builder->get_widget("render_mode_wireframe", mWireframeButton);

    mLightingButton->signal_toggled().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::onModeChange), mLightingButton));
    mDepthButton->signal_toggled().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::onModeChange), mDepthButton));
    mTextureButton->signal_toggled().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::onModeChange), mTextureButton));
    mSegmentationButton->signal_toggled().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::onModeChange), mSegmentationButton));
    mWireframeButton->signal_toggled().connect(sigc::bind(
        sigc::mem_fun(*this, &MainWindow::onModeChange), mWireframeButton));
  }

  void onModeChange(Gtk::RadioButton *button) {
    // do not care unselected
    if (!button->get_active()) {
      return;
    }
    // TODO: race
    if (!gContext) {
      return;
    }

    if (button == mLightingButton) {
      gRenderQueue.push("lighting");
    } else if (button == mDepthButton) {
      gRenderQueue.push("depth");
    } else if (button == mTextureButton) {
      gRenderQueue.push("texture");
    } else if (button == mSegmentationButton) {
      gRenderQueue.push("segmentation");
    } else {
      std::cerr << "Wireframe not yet supported." << std::endl;
    }
  }

  virtual ~MainWindow() = default;
};

void click(int id) { printf("Click object %d\n", id); }

void enter(int id) { printf("Entering object %d\n", id); }

void leave(int id) { printf("Leaving object %d\n", id); }

int main(int argc, char **argv) {

  std::thread gtkThread([&] {
    auto app = Gtk::Application::create(argc, argv, "optifuser.example");
    auto builder = Gtk::Builder::create();
    builder->add_from_file("../assets/visualizer.glade");
    window = nullptr;
    builder->get_widget_derived("MainWindow", window);
    auto r = app->run(*window);
    delete window;
    return r;
  });

  // set up GLFW stuff
  int w = 1080;
  int h = 720;
  Optifuser::GLFWRenderContext &context =
      Optifuser::GLFWRenderContext::Get(w, h);
  gContext = &context;
  Optifuser::Scene scene;

  loadScene(scene);

  Optifuser::FPSCameraSpec cam;
  cam.setUp({0,1,0});
  cam.setForward({0, 0, -1});
  CHECK(cam.isSane(), "Camera axes are wrong.");

  cam.position = {0, 0, 1};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  cam.rotation = cam.getRotation0();
  scene.addDirectionalLight(
      {glm::vec3(0.1, -1, -0.5), glm::vec3(0.5, 0.5, 0.5)});
  // scene->addPointLight({glm::vec3(0, 1, 0), glm::vec3(0.5, 0.5, 0.5)});
  scene.setAmbientLight(glm::vec3(0.05, 0.05, 0.05));

  scene.setEnvironmentMap("../assets/ame_desert/desertsky_ft.tga",
                          "../assets/ame_desert/desertsky_bk.tga",
                          "../assets/ame_desert/desertsky_up.tga",
                          "../assets/ame_desert/desertsky_dn.tga",
                          "../assets/ame_desert/desertsky_lf.tga",
                          "../assets/ame_desert/desertsky_rt.tga");

  context.renderer.setShadowShader("../glsl_shader/shadow.vsh",
                                   "../glsl_shader/shadow.fsh");
  context.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                    "../glsl_shader/gbuffer_segmentation.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                     "../glsl_shader/deferred.fsh");
  context.renderer.setAxisShader("../glsl_shader/axes.vsh",
                                 "../glsl_shader/axes.fsh");

  GLuint pickingFbo;
  glGenFramebuffers(1, &pickingFbo);

  std::string mode = "lighting";
  int hoveredId = 0;
  int leftPressed = 0;
  while (true) {
    while (!gRenderQueue.empty()) {
      std::string msg = gRenderQueue.pop();
      std::cerr << msg << " received." << std::endl;
      if (msg == "lighting") {
        mode = "lighting";
        context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                           "../glsl_shader/deferred.fsh");
      } else if (msg == "depth") {
        mode = "depth";
        context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                           "../glsl_shader/deferred_depth.fsh");
      } else if (msg == "texture") {
        mode = "texture";
        context.renderer.setDeferredShader(
            "../glsl_shader/deferred.vsh",
            "../glsl_shader/deferred_albedo.fsh");
      } else if (msg == "segmentation") {
        mode = "segmentation";
      }
    }

    context.processEvents();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }
    cam.aspect = context.getWidth() / (float)context.getHeight();

    constexpr float dt = 0.05f;
    float f = 0.f;
    float r = 0.f;
    if (Optifuser::getInput().getKeyState(GLFW_KEY_W)) {
      f += dt;
    }
    if (Optifuser::getInput().getKeyState(GLFW_KEY_S)) {
      f -= dt;
    }
    if (Optifuser::getInput().getKeyState(GLFW_KEY_A)) {
      r -= dt;
    }
    if (Optifuser::getInput().getKeyState(GLFW_KEY_D)) {
      r += dt;
    }
    cam.moveForwardRight(f, r);

    if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) ==
        GLFW_PRESS) {
      double dx, dy;
      Optifuser::getInput().getCursorDelta(dx, dy);
      cam.rotateYawPitch(-dx / 1000.f, -dy / 1000.f);
    }

    int x, y;
    Optifuser::getInput().getCursor(x, y);

    bool leftClick = false;
    if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) ==
        GLFW_PRESS) {
      if (!leftPressed) {
        leftClick = true;
        leftPressed = true;
      }
    } else {
      leftPressed = false;
    }
    if (context.renderer.segtex[0]) {
      if (x >= 0 && x < context.getWidth() && y >= 0 &&
          y < context.getHeight()) {
        int value;
        glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, context.renderer.segtex[0], 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        // pixel position is upside down
        glReadPixels(x, context.getHeight() - y, 1, 1, GL_RED_INTEGER, GL_INT,
                     &value);
        if (value && leftClick) {
          click(value);
          context.renderer.setObjectIdForAxis(value);
        }

        if (value != hoveredId) {
          if (hoveredId) {
            leave(hoveredId);
          }
          if (value) {
            enter(value);
          }
          hoveredId = value;
        }
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    context.renderer.renderScene(scene, cam);
    // context.renderer.saveLighting("lighting.raw");
    // context.renderer.saveNormal("normal.raw");
    // context.renderer.saveDepth("depth.raw");

    if (mode == "segmentation") {
      context.renderer.displaySegmentation();
    } else {
      context.renderer.displayLighting();
    }
    context.swapBuffers();
  }

  context.destroy();
}
