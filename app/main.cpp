#include "camera_spec.h"
#include "objectLoader.h"
#include "optifuser.h"
#include "renderer.h"
#include "scene.h"
#include <experimental/filesystem>
#include <iostream>
using std::cout;
using std::endl;
namespace fs = std::experimental::filesystem;

void loadSponza(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj");
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene.addObject(std::move(obj));
  }
}

int main() {
  int w = 1080;
  int h = 720;
  Optifuser::GLFWRenderContext &context =
      Optifuser::GLFWRenderContext::Get(w, h);
  Optifuser::Scene scene;

  std::vector<std::shared_ptr<Optifuser::Object>> objects;

  Optifuser::FPSCameraSpec cam;
  cam.up = {0, 0, 1};
  cam.forward = {1, 0, 0};
  cam.position = {-3, 0, 0};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  scene.addDirectionalLight({glm::vec3(0, 0, -1), glm::vec3(0.5, 0.5, 0.5)});
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

    float dt = 0.05f;
    if (Optifuser::getInput().getKeyState(GLFW_KEY_W)) {
      cam.moveForwardRight(dt, 0);
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_S)) {
      cam.moveForwardRight(-dt, 0);
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_A)) {
      cam.moveForwardRight(0, -dt);
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_D)) {
      cam.moveForwardRight(0, dt);
    }

    if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) ==
        GLFW_PRESS) {
      double dx, dy;
      Optifuser::getInput().getCursor(dx, dy);
      cam.rotateYawPitch(-dx / 1000.f, -dy / 1000.f);
    }
    context.render(scene, cam);
  }
}
