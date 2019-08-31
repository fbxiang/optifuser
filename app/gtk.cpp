#include "objectLoader.h"
#include "optifuser.h"
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
  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj");
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene.addObject(std::move(obj));
  }
}

class MainWindow : public Gtk::ApplicationWindow {
private:
  std::unique_ptr<Optifuser::Renderer> mRenderer;
  std::shared_ptr<Optifuser::Scene> mScene;
  Optifuser::FPSCameraSpec mCamera;
  Glib::RefPtr<Gtk::Builder> builder;

public:
  MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const &builder)
      : Gtk::ApplicationWindow(obj), builder{builder} {}

  virtual ~MainWindow() = default;
};

int main(int argc, char **argv) {

  std::thread gtkThread([&] {
    auto app = Gtk::Application::create(argc, argv, "optifuser.example");
    auto builder = Gtk::Builder::create();
    builder->add_from_file("../assets/visualizer.glade");
    MainWindow *window = nullptr;
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
  Optifuser::Scene scene;

  loadSponza(scene);

  Optifuser::FPSCameraSpec cam;
  cam.up = {0, 0, 1};
  cam.forward = {0, 1, 0};
  CHECK(cam.isSane(), "Camera axes are wrong.");

  cam.position = {0, -1, 0};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  cam.rotation = cam.getRotation0();
  scene.addDirectionalLight({glm::vec3(0, -1, -1), glm::vec3(0.5, 0.5, 0.5)});
  // scene->addPointLight({glm::vec3(0, 1, 0), glm::vec3(0.5, 0.5, 0.5)});
  scene.setAmbientLight(glm::vec3(0.05, 0.05, 0.05));

  scene.setEnvironmentMap("../assets/ame_desert/desertsky_ft.tga",
                          "../assets/ame_desert/desertsky_bk.tga",
                          "../assets/ame_desert/desertsky_up.tga",
                          "../assets/ame_desert/desertsky_dn.tga",
                          "../assets/ame_desert/desertsky_lf.tga",
                          "../assets/ame_desert/desertsky_rt.tga");

  context.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                    "../glsl_shader/gbuffer.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                     "../glsl_shader/deferred.fsh");

  while (true) {
    context.processEvents();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }

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
    context.render(scene, cam);
  }

  context.destroy();
  gtkThread.join();
}
