#include "prd.h"
#include "random.h"
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

// Scene
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(rtObject, top_object, , );

rtDeclareVariable(unsigned int, pathtrace_ray_type, , );
rtDeclareVariable(unsigned int, pathtrace_shadow_ray_type, , );
rtDeclareVariable(float3, bad_color, , );
rtDeclareVariable(unsigned int, n_samples_sqrt, , );
rtDeclareVariable(unsigned int, iterations, , );
rtDeclareVariable(unsigned int, n_rays, , );

rtBuffer<float4, 2> output_buffer;
rtBuffer<float4, 2> albedo_buffer;
rtBuffer<float4, 2> normal_buffer;

rtDeclareVariable(Matrix3x3, normal_matrix, , );

// Camera
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );

// Ray
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(PerRayData, current_prd, rtPayload, );

RT_PROGRAM void camera() {
  size_t2 screen = output_buffer.size();
  float2 inv_screen = 1.f / make_float2(screen) * 2.f;
  float2 pixel = make_float2(launch_index) * inv_screen - 1.f;

  float3 result = make_float3(0.f);
  float3 albedo = make_float3(0.f);
  float3 normal = make_float3(0.f);

  unsigned int seed = tea<16>(screen.x * launch_index.y + launch_index.x, iterations);

  const unsigned int n_samples = n_samples_sqrt * n_samples_sqrt;
  unsigned int n_samples2 = n_samples;

  do {
    float r1, r2;
    halton2d(r1, r2, n_samples2 + iterations * n_samples, 3, 5);

    // pixel is in range [-1, 1) x [-1, 1)
    float2 d = pixel + make_float2(r1, r2) * inv_screen;
    float3 ray_origin = eye;
    float3 ray_direction = normalize(d.x * U + d.y * V + W);

    PerRayData prd;
    prd.albedo = make_float3(0.f);
    prd.normal = make_float3(0.f);
    prd.radiance = make_float3(0.f);
    prd.direction = ray_direction;
    prd.result = make_float3(0.f);
    prd.attenuation = make_float3(1.f);
    prd.done = false;
    prd.depth = 0;
    prd.seed = seed;
    prd.max_depth_override = 0;

    for (;;) {
      float3 attenuation = prd.attenuation;
      Ray ray =
          make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
      rtTrace(top_object, ray, prd);
      prd.result += prd.radiance * attenuation;

      if (prd.done) {
        break;
      }
      prd.depth++;

      if (prd.depth >= prd.max_depth_override && prd.depth >= n_rays) {
        break;
      }

      ray_origin = prd.origin;
      ray_direction = prd.direction;
    }
    result += prd.result;
    albedo += prd.albedo;

    float3 normal_eyespace = (length(prd.normal) > 0.f) ? normalize(normal_matrix * prd.normal)
                                                        : make_float3(0., 0., 1.);
    normal += normal_eyespace;
    seed = prd.seed;

  } while (--n_samples2);

  result /= n_samples;
  albedo /= n_samples;
  normal = normalize(normal);

  float r = 1.f / (iterations + 1);

  output_buffer[launch_index] =
      output_buffer[launch_index] * (1 - r) + make_float4(result, 1.f) * r;
  albedo_buffer[launch_index] =
      albedo_buffer[launch_index] * (1 - r) + make_float4(albedo, 1.f) * r;
  normal_buffer[launch_index] =
      normal_buffer[launch_index] * (1 - r) + make_float4(normal, 1.f) * r;
}

RT_PROGRAM void exception() { output_buffer[launch_index] = make_float4(bad_color, 1.f); }
