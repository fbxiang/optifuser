#include <optix_device.h>
#include <optix_math.h>

#include "prd.h"

rtDeclareVariable(int, envmapId, , );
rtDeclareVariable(PerRayData, current_prd, rtPayload, );

RT_PROGRAM void miss() {
  current_prd.radiance = make_float3(rtTexCubemap<float4>(
      envmapId, -current_prd.direction.y, current_prd.direction.z, -current_prd.direction.x));
  current_prd.done = 1;
}
