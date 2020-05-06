#include "camera_spec.h"
#include "objectLoader.h"
#include "optifuser.h"
#include "partnet_loader.hpp"
#include "renderer.h"
#include "scene.h"
#include <experimental/filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using std::cout;
using std::endl;
namespace fs = std::experimental::filesystem;

enum RenderMode { LIGHTING, ALBEDO, NORMAL, DEPTH, SEGMENTATION };

void loadSponza(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("../scenes/sponza/sponza.obj", true, {0, 0, 1}, {0, 1, 0});
  for (auto &obj : objects) {
    obj->scale = glm::vec3(0.003f);
    obj->position *= 0.003f;
    scene.addObject(std::move(obj));
  }
}

Optifuser::Object *loadDragon(Optifuser::Scene &scene) {
  auto objects = Optifuser::LoadObj("../assets/dragon.obj");
  auto obj = objects[0].get();
  scene.addObject(std::move(objects[0]));
  return obj;
}

int main() {
  int w = 1080;
  int h = 720;
  Optifuser::GLFWRenderContext &context = Optifuser::GLFWRenderContext::Get(w, h);
  context.initGui();

  Optifuser::Scene scene;
  int renderMode = RenderMode::LIGHTING;

  std::vector<std::shared_ptr<Optifuser::Object>> objects;

  Optifuser::FPSCameraSpec cam;
  cam.setUp({0, 1, 0});
  cam.setForward({0, 0, -1});
  cam.position = {0, 0, 3};
  cam.fovy = glm::radians(45.f);
  cam.aspect = w / (float)h;
  cam.setRotation(cam.getRotation0());
  // cam.rotateYawPitch(glm::radians(-90.f), 0);

  // loadSponza(scene);
  scene.addDirectionalLight({glm::vec3(0, 0, -1), glm::vec3(0.5, 0.5, 0.5)});
  scene.setAmbientLight(glm::vec3(0.05, 0.05, 0.05));

  loadSponza(scene);
  // auto dragon = loadDragon(scene);
  // dragon->pbrMaterial->kd = {1, 0, 0, 1};
  // scene.setEnvironmentMap("../assets/ame_desert/desertsky_ft.tga",
  //                          "../assets/ame_desert/desertsky_bk.tga",
  //                          "../assets/ame_desert/desertsky_up.tga",
  //                          "../assets/ame_desert/desertsky_dn.tga",
  //                          "../assets/ame_desert/desertsky_lf.tga",
  //                          "../assets/ame_desert/desertsky_rt.tga");

  context.renderer.enableDisplayPass();
  // context.renderer.enableAOPass();
  // context.renderer.enableShadowPass();

  context.renderer.setShadowShader("../glsl_shader/shadow.vsh", "../glsl_shader/shadow.fsh");
  context.renderer.setGBufferShader("../glsl_shader/gbuffer.vsh",
                                    "../glsl_shader/gbuffer_segmentation.fsh");
  context.renderer.setAOShader("../glsl_shader/ssao.vsh",
                                    "../glsl_shader/ssao.fsh");
  context.renderer.setDeferredShader("../glsl_shader/deferred.vsh", "../glsl_shader/deferred.fsh");
  context.renderer.setAxisShader("../glsl_shader/axes.vsh", "../glsl_shader/axes.fsh");
  context.renderer.setTransparencyShader("../glsl_shader/transparency.vsh",
                                         "../glsl_shader/transparency.fsh");
  context.renderer.setDisplayShader("../glsl_shader/display.vsh", "../glsl_shader/display_normal.fsh");
  context.renderer.setCompositeShader("../glsl_shader/composite.vsh", "../glsl_shader/composite.fsh");

  context.showWindow();

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

    if (Optifuser::getInput().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      double dx, dy;
      Optifuser::getInput().getCursorDelta(dx, dy);
      cam.rotateYawPitch(-dx / 1000.f, -dy / 1000.f);
    }
    context.renderer.renderScene(scene, cam);
    if (renderMode == LIGHTING) {
      context.renderer.displayLighting();
    } else if (renderMode == SEGMENTATION) {
      context.renderer.displaySegmentation();
    } else {
      context.renderer.display();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Render Options");
    {
      if (ImGui::CollapsingHeader("Render Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::RadioButton("Lighting", &renderMode, RenderMode::LIGHTING)) {
        };
        if (ImGui::RadioButton("Albedo", &renderMode, RenderMode::ALBEDO)) {
          context.renderer.setDisplayShader("../glsl_shader/display.vsh", "../glsl_shader/display_albedo.fsh");
        }
        if (ImGui::RadioButton("Normal", &renderMode, RenderMode::NORMAL)) {
          context.renderer.setDisplayShader("../glsl_shader/display.vsh", "../glsl_shader/display_normal.fsh");
        }
        if (ImGui::RadioButton("Depth", &renderMode, RenderMode::DEPTH)) {
          context.renderer.setDisplayShader("../glsl_shader/display.vsh", "../glsl_shader/display_depth.fsh");
        }
        ImGui::RadioButton("Segmentation", &renderMode, RenderMode::SEGMENTATION);
      }

      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Position");
        ImGui::Text("%-4.3f %-4.3f %-4.3f", cam.position.x, cam.position.y, cam.position.z);
        ImGui::Text("Forward");
        auto forward = cam.getRotation() * glm::vec3(0, 0, -1);
        ImGui::Text("%-4.3f %-4.3f %-4.3f", forward.x, forward.y, forward.z);
      }

      if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Frame Time: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
      }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    context.swapBuffers();
  }
}
