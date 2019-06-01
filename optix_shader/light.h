#pragma once
#include <optix_world.h>

using namespace optix;

struct DirectionalLight {
  float3 direction;
  float3 emission;
};

struct PointLight {
  float3 position;
  float3 emission;
};

struct ParallelogramLight {
  float3 corner;
  float3 v1, v2;
  float3 normal;
  float3 emission;
};
