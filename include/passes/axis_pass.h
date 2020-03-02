#include "gbuffer_pass.h"

namespace Optifuser {

class AxisPass : public GBufferPass {
private:
  int objId = 0;

public:
  bool globalAxes = false;

  inline void setObjectId(int id) { objId = id; }
  void render(const Scene &scene, const CameraSpec &camera);
};
} // namespace Optifuser
