#include "camera_spec.h"
#include "objectLoader.h"
#include "optifuser.h"
#include "optix_renderer.h"
#include "partnet_loader.hpp"
#include "scene.h"
#include <experimental/filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using std::cout;
using std::endl;
namespace fs = std::experimental::filesystem;

GLFWwindow *window;

void loadPartNetModel(Optifuser::Scene &scene) {
  auto files = fs::directory_iterator("../assets/7128/textured_objs");
  for (auto &f : files) {
    if (f.path().extension() == ".obj") {
      auto objects = Optifuser::LoadObj(f.path().u8string());
      for (auto &obj : objects) {
        scene.addObject(std::move(obj));
      }
    }
  }
}

void loadSponza(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj", true, {0, 0, 1}, {1, 0, 0});
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene.addObject(std::move(obj));
  }
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

  cam.position = {-1, 0, 0};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  cam.setRotation(cam.getRotation0());

  loadSponza(scene);
  // loadPartNetModel(scene);

  // scene.addDirectionalLight({glm::vec3(0, -1, 0.1), glm::vec3(1, 1, 1)});
  // scene.addDirectionalLight({glm::vec3(0, 0, -1), glm::vec3(1, 1, 1)});
  // scene.setAmbientLight(glm::vec3(0.05, 0.05, 0.05));

  scene.addParalleloGramLight({{-5, 5, 3}, {10, 0, 0}, {0, -10, 0}, {0, 0, -1}, {0, 1, 1}});

  globalContext.initGui();
  globalContext.showWindow();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_FRAMEBUFFER_SRGB_EXT);

  optixContext->renderer.numRays = 4;
  optixContext->renderer.max_iterations = 100000;
  // optixContext->renderer.setHdrmap("/home/fx/textures/artist_workshop_4k.hdr");

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
