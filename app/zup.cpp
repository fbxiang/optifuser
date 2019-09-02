#include "camera_spec.h"
#include "objectLoader.h"
#include "optifuser.h"
#include "renderer.h"
#include "scene.h"
#include <iostream>

using std::cout;
using std::endl;

#define CHECK(exp, msg)                                                        \
  {                                                                            \
    if (!exp) {                                                                \
      std::cerr << msg << std::endl;                                           \
      exit(1);                                                                 \
    }                                                                          \
  }

int main() {
  int w = 1080;
  int h = 720;
  Optifuser::GLFWRenderContext &context =
      Optifuser::GLFWRenderContext::Get(w, h);
  Optifuser::Scene scene;

  uint32_t id = 0;
  auto objects = Optifuser::LoadObj(
      "/home/fx/source/physx-project/assets/robot/movo_description/meshes/"
      "manipulation/jaco/visual/base.dae");
  for (auto &obj : objects) {
    obj->setSegmentId(++id);
    scene.addObject(std::move(obj));
  }

  objects = Optifuser::LoadObj(
      "/home/fx/source/physx-project/assets/robot/movo_description/meshes/"
      "manipulation/jaco/visual/shoulder_7dof.dae");
  for (auto &obj : objects) {
    obj->setSegmentId(++id);
    scene.addObject(std::move(obj));
  }

  // auto axes = Optifuser::NewAxes();
  // axes->scale = {0.1, 0.1, 0.1};
  // scene->addObject(std::move(axes));

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
                                    "../glsl_shader/gbuffer_segmentation.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                     "../glsl_shader/deferred.fsh");
  context.renderer.renderSegmentation();

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
    context.renderer.renderScene(scene, cam);
    context.renderer.displayLighting();
    context.swapBuffers();
  }
}
