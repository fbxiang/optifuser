#pragma once
#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include "random.h"

using namespace optix;

// ================================================================================
// Diffuse
static __host__ __device__ __inline__ float3 ForwardDiffuse(float3 wi, float3 wo, float3 normal, float3 color) {
  return color * fmaxf(dot(wi, normal), 0.f);
}

static __host__ __device__ __inline__ void SampleDiffuse(float3 wo, float3 normal, float3 color,
                   float3& wi, float3& reflectance, unsigned int&seed) {
  float z1 = rnd(seed);
  float z2 = rnd(seed);
  cosine_sample_hemisphere(z1, z2, wi);
  Onb onb(normal);
  onb.inverse_transform(wi);
  reflectance = color;
}

// ================================================================================
// GGX

static __host__ __device__ __inline__ float SmithGGXMaskingShadowing(float3 wi, float3 wo,
                                                                     float3 normal, float a2)
{
  float dotNL = dot(normal, wi);
  float dotNV = dot(normal, wo);

  float denomA = dotNV * sqrtf(a2 + (1.0f - a2) * dotNL * dotNL);
  float denomB = dotNL * sqrtf(a2 + (1.0f - a2) * dotNV * dotNV);

  return 2.0f * dotNL * dotNV / (denomA + denomB);
}

static __host__ __device__ __inline__ void SampleGGX_ImpD(float3 wo, float3 normal, float roughness, float ks,
                                                          float3 color, float3& wi, float3& reflectance,
                                                          unsigned int& seed) {
  float F0 = ks;
  float a2 = roughness * roughness;

  // sample microfacet normal direction
  float z1 = rnd(seed);
  float z2 = rnd(seed);
  float theta = acosf(sqrtf((1.0f - z1) / ((a2 - 1.0f) * z1 + 1.0f)));
  float phi   = 2.f * M_PIf  * z2;
  float3 wm = make_float3(cosf(phi) * sinf(theta), sinf(phi) * sinf(theta), cosf(theta));
  Onb onb(normal);
  onb.inverse_transform(wm);

  // -- Calculate wi by reflecting wo about wm
  wi = -reflect(wo, wm);

  // -- Ensure our sample is in the upper hemisphere
  // -- Since we are in tangent space with a y-up coordinate
  // -- system BsdfNDot(wi) simply returns wi.y
  if(dot(normal, wi) > 0.0f && dot(wi, wm) > 0.0f) {

    // -- calculate the reflectance to multiply by the energy
    // -- retrieved in direction wi
    // float F = fresnel_schlick(dot(wi, wm), 5.f, F0, 1.f);
    float F = F0;
    float G = SmithGGXMaskingShadowing(wi, wo, normal, a2);
    float weight = fabsf(dot(wo, wm))
                   / (dot(normal, wo) * dot(normal, wm));

    reflectance = color * F * G * weight;
  } else {
    reflectance = make_float3(0.f);
  }
}

static __host__ __device__ __inline__ float3 ForwardGGX(float3 wi, float3 wo, float3 normal, float roughness,
                                                        float ks, float3 color)
{
  float F0 = ks;
  float a2 = roughness * roughness;

  if (dot(wi, normal) > 0 && dot(wo, normal) > 0) {
    float3 wm = normalize((wi + wo) / 2);
    // float F = fresnel_schlick(dot(wi, wm), 5.f, F0, 1);
    float F = F0;
    float G = SmithGGXMaskingShadowing(wi, wo, normal, a2);
    float D_d = dot(normal, wm) * dot(normal, wm) * (a2 - 1) + 1;
    float D = a2 / (M_PIf * D_d * D_d);
    return color * F * G * D / (4 * dot(wi, normal) * dot(wo, normal));
  } else {
    return make_float3(0.f);
  }
}
