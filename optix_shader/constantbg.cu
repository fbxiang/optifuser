#include <optix_device.h>

#include "prd.h"

rtDeclareVariable(float3,     bg_color,    ,          );
rtDeclareVariable(PerRayData, current_prd, rtPayload, );

RT_PROGRAM void miss() {
  current_prd.result = bg_color;
  current_prd.done = 1;
}
