#include "input.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
namespace Optifuser {
void Input::keyCallback(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS)
    keyState[key] = 1;
  else if (action == GLFW_RELEASE)
    keyState[key] = 0;
}

int Input::getKeyState(int key) const {
  auto k = keyState.find(key);
  if (k != keyState.end()) {
    return k->second;
  }
  return 0;
}

void Input::cursorPosCallback(double x, double y) {
  dx = x - lastX;
  dy = y - lastY;
  lastX = x;
  lastY = y;
}

void Input::getCursor(double &dx, double &dy) {
  dx = this->dx;
  dy = this->dy;
  if (dx > 100)
    dx = 100;
  if (dy > 100)
    dy = 100;
  if (firstTime) {
    firstTime = false;
    dx = dy = 0;
  }
}

void Input::mouseCallback(int button, int state) { mouseState[button] = state; }

int Input::getMouseButton(int button) { return mouseState[button]; }
} // namespace Optifuser
