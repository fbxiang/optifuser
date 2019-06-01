#include "light.h"
#include "prd.h"
#include "random.h"
#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>


rtDeclareVariable(rtObject,          top_object,                ,                           );
rtDeclareVariable(float,             scene_epsilon,             ,                           );
rtDeclareVariable(unsigned int,      pathtrace_shadow_ray_type, ,                           );

rtBuffer<DirectionalLight>    directional_lights;
rtBuffer<PointLight>          point_lights;
rtBuffer<ParallelogramLight>  parallelogram_lights;

rtDeclareVariable(optix::Ray,        ray,                       rtCurrentRay,               );
rtDeclareVariable(float3,            hit_point,                 attribute hit_point,        ); 

rtDeclareVariable(int,               use_shadow,                ,                           );

rtTextureSampler<float4,             2> kd_map;


rtDeclareVariable(PerRayData,        current_prd,               rtPayload,                  );
rtDeclareVariable(PerRayData_shadow, prd_shadow,                rtPayload,                  );
rtDeclareVariable(float3,            geometric_normal,          attribute geometric_normal, ); 
rtDeclareVariable(float3,            shading_normal,            attribute shading_normal,   ); 
rtDeclareVariable(float,             t_hit,                     rtIntersectionDistance,     );


RT_PROGRAM void closest_hit() {
  float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 ffnormal               = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );
  float3 hitpoint               = rtTransformPoint(RT_OBJECT_TO_WORLD, hit_point);

  current_prd.origin = hitpoint;
  float3 p = reflect(normalize(ray.direction), normalize(ffnormal));
  current_prd.direction = p;
  current_prd.attenuation *= 1;
  current_prd.max_depth_override = 2;
  current_prd.radiance = make_float3(0.f);
}

RT_PROGRAM void any_hit() {
}

RT_PROGRAM void shadow_any_hit() {
  if (!use_shadow) {
    rtTerminateRay();
    return;
  }

  prd_shadow.inShadow = true;
  rtTerminateRay();
}
