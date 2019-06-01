#include "../renderer/objectLoader.h"
#include "../renderer/optifuser.h"
#include "../renderer/renderer.h"
#include "../renderer/scene.h"
#include <iostream>
using std::cout;
using std::endl;

int main() {
  int w = 640;
  int h = 480;
  Optifuser::GLFWRenderContext context(w, h);
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

  context.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                    "../glsl_shader/gbuffer.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh",
                                     "../glsl_shader/deferred.fsh");
  int frame = 0;
  while (1) {
    context.render(scene);
    ++frame;
    // if (frame % 60 == 0) {
    //   context.renderer.reloadShaders();
    // }
  }
}
