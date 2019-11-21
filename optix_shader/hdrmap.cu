#include <optix_device.h>
#include <optix_math.h>

#include "prd.h"

rtTextureSampler<float4, 2> envmap;
rtDeclareVariable(PerRayData, current_prd, rtPayload, );
RT_PROGRAM void miss() {
  // float theta = atan2f(current_prd.direction.x, current_prd.direction.z);
  float theta = atan2f(-current_prd.direction.y, -current_prd.direction.x);
  float phi = M_PIf * 0.5f - acosf(current_prd.direction.z);
  float u = (theta + M_PIf) * (0.5f * M_1_PIf);
  float v = 0.5f * (1.0f + sin(phi));
  current_prd.radiance = make_float3(tex2D(envmap, u, 1-v));
  current_prd.done = 1;
}
