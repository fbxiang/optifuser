#pragma once
#include <optix_world.h>

using namespace optix;

struct PerRayData {
  float3 result;
  float3 albedo;
  float3 normal;
  float3 radiance;
  float3 attenuation;
  float3 origin;
  float3 direction;
  int depth;
  int max_depth_override;
  int done;
  unsigned int seed;
};

struct PerRayData_shadow {
  float3 attenuation;
  bool inShadow;
};
