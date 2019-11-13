#include <optix_device.h>
#include <optix_math.h>

#include "prd.h"

rtDeclareVariable(PerRayData, current_prd, rtPayload, );

__device__ float3 procedural_color(float angle) {
  float3 horizonColor = .4f * make_float3(0.9, 0.9, 0.9);
  float3 zenithColor = .4f * make_float3(0.522, 0.757, 0.914);
  float3 groundColor = .4f * make_float3(0.5, 0.410, 0.271);

  return lerp(lerp(zenithColor, horizonColor, smoothstep(15.f, 5.f, angle)), groundColor,
             smoothstep(-5.f, -15.f, angle));
}

RT_PROGRAM void miss() {
  float phi = asinf(current_prd.direction.z);
  current_prd.result = procedural_color(phi * 57.3);
  current_prd.done = 1;
}
