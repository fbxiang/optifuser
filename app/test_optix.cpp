#include "camera_spec.h"
#include "objectLoader.h"
#include "optifuser.h"
#include "optix_renderer.h"
#include "partnet_loader.hpp"
#include "scene.h"
#include <experimental/filesystem>
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using std::cout;
using std::endl;
namespace fs = std::experimental::filesystem;

GLFWwindow *window;

void loadSponza(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("/home/fx/Scenes/McGuire_sponza/sponza.obj", true, {0, 0, 1}, {1, 0, 0});
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene.addObject(std::move(obj));
  }
}

Optifuser::Object* loadDragon(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("../assets/dragon.obj", true, {0,0,1}, {1,0,0});
  auto obj = objects[0].get();
  scene.addObject(std::move(objects[0]));
  return obj;
}

Optifuser::Object* loadCube(Optifuser::Scene &scene) {
  auto cube = Optifuser::NewFlatCube();
  auto obj = cube.get();
  scene.addObject(std::move(cube));
  return obj;
}

int main() {
  int w = 640;
  int h = 480;

  auto &globalContext = Optifuser::GLFWRenderContext::Get();
  auto optixContext = Optifuser::OptixContext::Create(w, h);

  Optifuser::Scene scene;
  Optifuser::FPSCameraSpec cam;
  cam.setUp({0, 0, 1});
  cam.setForward({1, 0, 0});

  cam.position = {-3, 0, 2};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  cam.setRotation(cam.getRotation0());

  loadSponza(scene);
  // auto dragon = loadDragon(scene);
  // dragon->pbrMaterial->kd = {0.85, 0.46, 0.34,1};
  // dragon->pbrMaterial->roughness = 0.1f;
  // dragon->pbrMaterial->ks = 0.8f;
  // dragon->pbrMaterial->metallic = 1.f;

  // dragon = loadDragon(scene);
  // dragon->pbrMaterial->kd = {0.85, 0.46, 0.34,1};
  // dragon->pbrMaterial->roughness = 0.1f;
  // dragon->pbrMaterial->ks = 0.8f;
  // dragon->pbrMaterial->metallic = 0.f;
  // dragon->position = {1, 0, 0};

  // auto dragon = loadCube(scene);
  // dragon->pbrMaterial->kd = {1,0,0,1};
  // dragon->pbrMaterial->roughness = 1.f;
  // dragon->pbrMaterial->metallic = 0.f;
  // dragon->pbrMaterial->metallic = 0.f;

  // dragon = loadCube(scene);
  // dragon->pbrMaterial->kd = {1,0,0,1};
  // dragon->pbrMaterial->roughness = 0.1f;
  // dragon->pbrMaterial->metallic = 1.f;
  // dragon->position = {2.5, 0, 0};

  scene.addPointLight({ {0,0,1}, {1,1,1} } );
  // scene.addDirectionalLight({glm::vec3(0.1, 0, -1), glm::vec3(.5, .5, .5)});
  // scene.setAmbientLight(glm::vec3(0.05, 0.05, 0.05));
  // scene.addParalleloGramLight({{-5, 5, 3}, {10, 0, 0}, {0, -10, 0}, {0, 0, -1}, {1, 1, 1}});

  globalContext.initGui();
  globalContext.showWindow();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_FRAMEBUFFER_SRGB_EXT);

  optixContext->renderer.setBlackBackground();
  optixContext->renderer.numRays = 4;
  optixContext->renderer.max_iterations = 100000;
  // optixContext->renderer.setHdrmap("../assets/railway_bridge_02_4k.hdr");
  optixContext->renderer.setProceduralSkyBackground();

  while (true) {
    globalContext.processEvents();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }

    float dt = 0.05f;
    if (Optifuser::getInput().getKeyState(GLFW_KEY_W)) {
      cam.moveForwardRight(dt, 0);
      optixContext->renderer.invalidateCamera();
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_S)) {
      cam.moveForwardRight(-dt, 0);
      optixContext->renderer.invalidateCamera();
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_A)) {
      cam.moveForwardRight(0, -dt);
      optixContext->renderer.invalidateCamera();
    } else if (Optifuser::getInput().getKeyState(GLFW_KEY_D)) {
      cam.moveForwardRight(0, dt);
      optixContext->renderer.invalidateCamera();
    }

    if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      double dx, dy;
      Optifuser::getInput().getCursorDelta(dx, dy);
      cam.rotateYawPitch(-dx / 1000.f, -dy / 1000.f);
      optixContext->renderer.invalidateCamera();
    }

    optixContext->renderer.renderScene(scene, cam);
    optixContext->renderer.display();
    globalContext.swapBuffers();
  }
}
