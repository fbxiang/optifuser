#include "objectLoader.h"
#include "optifuser.h"
#include "safe_queue.h"
#include <GL/glew.h>
#include <gdkmm-3.0/gdkmm.h>
#include <gtkmm-3.0/gtkmm.h>
#include <iostream>
#include <memory>
#include <thread>

#define CHECK(exp, msg)                                                        \
  {                                                                            \
    if (!exp) {                                                                \
      std::cerr << msg << std::endl;                                           \
      exit(1);                                                                 \
    }                                                                          \
  }

void loadSponza(Optifuser::Scene &scene) {
  // uint32_t id = 0;
  // auto objects = Optifuser::LoadObj(
  //     "/home/fx/source/physx-project/assets/robot/movo_description/meshes/"
  //     "manipulation/jaco/visual/base.dae");
  // for (auto &obj : objects) {
  //   obj->setSegmentId(++id);
  //   scene.addObject(std::move(obj));
  // }

  // objects = Optifuser::LoadObj(
  //     "/home/fx/source/physx-project/assets/robot/movo_description/meshes/"
  //     "manipulation/jaco/visual/shoulder_7dof.dae");
  // for (auto &obj : objects) {
  //   obj->setSegmentId(++id);
  //   scene.addObject(std::move(obj));
  // }

  int id = 0;
  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj");
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    obj->setSegmentId(++id);
    scene.addObject(std::move(obj));
  }

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

  loadSponza(scene);

  Optifuser::FPSCameraSpec cam;
  cam.up = {0, 1, 0};
  cam.forward = {0, 0, -1};
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
  context.renderer.renderSegmentation(true);

  std::string mode = "lighting";
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
      Optifuser::getInput().getCursor(dx, dy);
      cam.rotateYawPitch(-dx / 1000.f, -dy / 1000.f);
    }
    context.renderer.renderScene(scene, cam);
    if (mode == "segmentation") {
      context.renderer.displaySegmentation();
    } else {
      context.renderer.displayLighting();
    }
    context.swapBuffers();
  }

  context.destroy();
}
