#include <map>
namespace Optifuser {
class Input {
  std::map<int, int> keyState;
  std::map<int, int> keyMods;
  std::map<int, int> mouseState;
  std::map<int, int> keyDown;
  std::map<int, int> mouseDown;

  double lastX = -1, lastY = -1, dx = 0, dy = 0, wdx = 0, wdy = 0;

  bool firstTime = true;

public:
  void keyCallback(int key, int scancode, int action, int mods);
  void cursorPosCallback(double x, double y);
  void wheelCallback(double x, double y);
  void mouseCallback(int button, int state);
  void nextFrame();

public:
  int getKeyState(int key) const;
  int getKeyMods(int key) const;
  int getKeyDown(int key) const;
  void getCursor(int &x, int &y);
  void getCursorDelta(double &dx, double &dy);
  void getWheelDelta(double &dx, double &dy);
  int getMouseButton(int button);
  int getMouseDown(int button);
};

} // namespace Optifuser
