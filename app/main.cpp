#include "objectLoader.h"
#include "optifuser.h"
#include "renderer.h"
#include "scene.h"
#include <iostream>
using std::cout;
using std::endl;

int main() {
  int w = 1080;
  int h = 720;
  Optifuser::GLFWRenderContext &context =
      Optifuser::GLFWRenderContext::Get(w, h);
  Optifuser::OffScreenRenderContext contextOffscreen(1080, 720);
  auto scene = std::make_shared<Optifuser::Scene>();

  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj");
  for (auto obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene->addObject(obj);
  }

  auto cam = Optifuser::NewObject<Optifuser::Camera>();
  cam->position = {1.5, 0.5, -0.5};
  cam->rotatePitch(-0.2);
  cam->rotateYaw(0.2);
  cam->rotateYaw(1.6);
  cam->rotatePitch(0.06);
  cam->fovy = glm::radians(45.f);
  cam->aspect = w / (float)h;
  scene->addObject(cam);
  scene->setMainCamera(cam);
  scene->addPointLight({glm::vec3(0, 1, 0), glm::vec3(0.5, 0.5, 0.5)});
  scene->setAmbientLight(glm::vec3(0.1, 0.1, 0.1));

  scene->setEnvironmentMap("../assets/ame_desert/desertsky_ft.tga",
                           "../assets/ame_desert/desertsky_bk.tga",
                           "../assets/ame_desert/desertsky_up.tga",
                           "../assets/ame_desert/desertsky_dn.tga",
                           "../assets/ame_desert/desertsky_lf.tga",
                           "../assets/ame_desert/desertsky_rt.tga");

  context.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                    "../glsl_shader/gbuffer.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                     "../glsl_shader/deferred.fsh");

  contextOffscreen.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                             "../glsl_shader/gbuffer.fsh");
  contextOffscreen.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                              "../glsl_shader/deferred.fsh");
  contextOffscreen.render(scene);
  contextOffscreen.save("/tmp/test.png");

  while (true) {
    context.processEvents();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }

    float dt = 0.05f;
    if (scene->getMainCamera()) {
      if (Optifuser::getInput().getKeyState(GLFW_KEY_W)) {
        scene->getMainCamera()->move(0, 0, 2 * dt);
      } else if (Optifuser::getInput().getKeyState(GLFW_KEY_S)) {
        scene->getMainCamera()->move(0, 0, -dt);
      } else if (Optifuser::getInput().getKeyState(GLFW_KEY_A)) {
        scene->getMainCamera()->move(0, -dt, 0);
      } else if (Optifuser::getInput().getKeyState(GLFW_KEY_D)) {
        scene->getMainCamera()->move(0, dt, 0);
      }

      if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) ==
          GLFW_PRESS) {
        double dx, dy;
        Optifuser::getInput().getCursor(dx, dy);
        scene->getMainCamera()->rotateYaw(-dx / 1000.f);
        scene->getMainCamera()->rotatePitch(-dy / 1000.f);
      }
    }
    context.render(scene);
  }
}
