#include "input.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
namespace Optifuser {
void Input::keyCallback(int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    keyState[key] = 1;
    keyMods[key] = mods;
    keyDown[key] = 1;
  } else if (action == GLFW_RELEASE) {
    keyState[key] = 0;
  }
}

int Input::getKeyMods(int key) const {
  auto m = keyMods.find(key);
  if (m != keyMods.end()) {
    return m->second;
  }
  return 0;
}

int Input::getKeyState(int key) const {
  auto k = keyState.find(key);
  if (k != keyState.end()) {
    return k->second;
  }
  return 0;
}

int Input::getKeyDown(int key) const {
  auto k = keyDown.find(key);
  if (k != keyDown.end()) {
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

void Input::wheelCallback(double x, double y) {
  wdx = x;
  wdy = y;
}

void Input::getCursor(int &x, int &y) {
  // Same as the resize function in renderer.cpp, scaling the size of input by 2
  // Start from 2012, all Macbook Pro use retina screen with high DPI
  // More information can be found:
  // https://support.apple.com/en-us/HT202471
  // It leads to the discrepency between pixel position and real position
  // IMGUI have already solve this problem, so don't worry for their code
  // More discussion can be found:
  // https://github.com/ocornut/imgui/issues/1676
  x = (int)lastX;
  y = (int)lastY;
#ifdef _USE_MACOSX
  x *= 2;
  y *= 2;
#endif
}

void Input::getCursorDelta(double &dx, double &dy) {
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

void Input::getWheelDelta(double &dx, double &dy) {
  dx = wdx;
  dy = wdy;
  wdx = 0;
  wdy = 0;
}

void Input::mouseCallback(int button, int state) {
  if (mouseState.find(button) == mouseState.end() ||
      (mouseState[button] != GLFW_PRESS && state == GLFW_PRESS)) {
    mouseDown[button] = 1;
  }

  mouseState[button] = state;
}

int Input::getMouseDown(int button) {
  auto k = mouseDown.find(button);
  if (k != mouseDown.end()) {
    return k->second;
  }
  return 0;
}
int Input::getMouseButton(int button) { return mouseState[button]; }

void Input::nextFrame() {
  keyDown.clear();
  mouseDown.clear();
}

} // namespace Optifuser
