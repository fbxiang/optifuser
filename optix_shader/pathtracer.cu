#include <optixu/optixu_math_namespace.h>
#include <optix_device.h>
#include "random.h"
#include "prd.h"

// Scene
rtDeclareVariable(float,        scene_epsilon,             ,              );
rtDeclareVariable(uint2,        launch_index,              rtLaunchIndex, );
rtDeclareVariable(rtObject,     top_object,                ,              );

rtDeclareVariable(unsigned int, pathtrace_ray_type,        ,              );
rtDeclareVariable(unsigned int, pathtrace_shadow_ray_type, ,              );
rtDeclareVariable(float3,       bad_color,                 ,              );
rtDeclareVariable(unsigned int, n_samples_sqrt,            ,              );
rtDeclareVariable(unsigned int, iterations,                ,              );
rtDeclareVariable(unsigned int, n_rays,                    ,              );

rtBuffer<float4, 2> output_buffer;


// Camera
rtDeclareVariable(float3 ,      eye ,                      ,              );
rtDeclareVariable(float3 ,      U   ,                      ,              );
rtDeclareVariable(float3 ,      V   ,                      ,              );
rtDeclareVariable(float3 ,      W   ,                      ,              );


// Ray
rtDeclareVariable(optix::Ray,   ray,                       rtCurrentRay,  );
rtDeclareVariable(PerRayData,   current_prd,               rtPayload,     );

RT_PROGRAM void camera() {
  size_t2 screen = output_buffer.size();
  float2 inv_screen = 1.f / make_float2(screen) * 2.f;
  float2 pixel = make_float2(launch_index) * inv_screen - 1.f;

  float3 result = make_float3(0.0f);

  unsigned int seed = tea<16>(screen.x*launch_index.y+launch_index.x, iterations);

  float2 jitter_scale = inv_screen / n_samples_sqrt;
  const unsigned int n_samples = n_samples_sqrt * n_samples_sqrt;
  unsigned int n_samples2 = n_samples;

  do {
    float r1, r2;
    halton2d(r1, r2, n_samples2 + iterations * n_samples, 3, 5);

    // pixel is in range [-1, 1) x [-1, 1)
    float2 d             = pixel + make_float2(r1, r2) * inv_screen;
    float3 ray_origin    = eye;
    float3 ray_direction = normalize(d.x * U + d.y * V + W);

    PerRayData prd;
    prd.radiance    = make_float3(0.f);
    prd.direction   = ray_direction;
    prd.result      = make_float3(0.f);
    prd.attenuation = make_float3(1.f);
    prd.done        = false;
    prd.depth       = 0;
    prd.seed        = seed;
    prd.max_depth_override = 0;

    for(;;) {
      Ray ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
      rtTrace(top_object, ray, prd);
      prd.result += prd.radiance * prd.attenuation;

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
    seed = prd.seed;

  } while (--n_samples2);

  result /= n_samples;

  float r = 1.f / (iterations+1);
  float4 color = make_float4(result, 1.f);

  output_buffer[launch_index] = output_buffer[launch_index] * (1-r) + color * r;
}


RT_PROGRAM void exception() {
  output_buffer[launch_index] = make_float4(bad_color, 1.f);
}
