#include "light.h"
#include "prd.h"
#include "random.h"
#include "shading_models.h"
#include <optix.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(unsigned int, pathtrace_shadow_ray_type, , );

rtBuffer<DirectionalLight> directional_lights;
rtBuffer<PointLight> point_lights;
rtBuffer<ParallelogramLight> parallelogram_lights;

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float4, kd, , );
rtDeclareVariable(float4, ks, , );
rtDeclareVariable(int, has_kd_map, , );
rtDeclareVariable(int, has_ks_map, , );
rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(float3, hit_point, attribute hit_point, );

rtDeclareVariable(int, use_shadow, , );

rtTextureSampler<float4, 2> kd_map;
rtTextureSampler<float4, 2> ks_map;

rtDeclareVariable(PerRayData, current_prd, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );

RT_PROGRAM void closest_hit() {
  float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
  float3 world_geometric_normal =
      normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
  float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);
  float3 hitpoint = rtTransformPoint(RT_OBJECT_TO_WORLD, hit_point);

  current_prd.max_depth_override = 0;

  float3 kd_val = make_float3(kd);
  float transmission = kd.w;
  if (has_kd_map) {
    float4 texture_kd_val = tex2D(kd_map, texcoord.x, texcoord.y);
    kd_val = make_float3(texture_kd_val);
    transmission = texture_kd_val.w;
  }

  float3 ks_val = make_float3(ks);
  float roughness = ks.w;
  if (has_ks_map) {
    float4 texture_ks_val = tex2D(ks_map, texcoord.x, texcoord.y);
    ks_val = make_float3(texture_ks_val);
    roughness = texture_ks_val.w;
  }

  float3 result = make_float3(0.f);
  if (rnd(current_prd.seed) < transmission) {
    current_prd.origin = hitpoint;

    float kd_l = luminance(kd_val);
    float ks_l = luminance(ks_val);

    float3 wi, reflectance;
    if (rnd(current_prd.seed) < kd_l / (kd_l + ks_l)) {
      // lambertian
      SampleDiffuse(-ray.direction, ffnormal, kd_val, wi, reflectance, current_prd.seed);
    } else {
      // GGX
      SampleGGX_ImpD(-ray.direction, ffnormal, roughness, ks_val, wi, reflectance, current_prd.seed);
    }
    current_prd.direction = wi;
    current_prd.attenuation *= reflectance;

    for (int i = 0; i < directional_lights.size(); i++) {
      DirectionalLight light = directional_lights[i];
      const float3 L = -light.direction;
      float3 reflectance = ForwardDiffuse(L, -ray.direction, ffnormal, kd_val) +
                           ForwardGGX(L, -ray.direction, ffnormal, roughness, ks_val);
      if (reflectance.x > 0.f && reflectance.y > 0.f && reflectance.z > 0.f) {
        PerRayData_shadow shadow_prd;
        shadow_prd.attenuation = make_float3(1.0f);
        shadow_prd.inShadow = false;
        Ray shadow_ray =
            make_Ray(hitpoint, -light.direction, pathtrace_shadow_ray_type, scene_epsilon, RT_DEFAULT_MAX);
        rtTrace(top_shadower, shadow_ray, shadow_prd);

        if (!shadow_prd.inShadow) {
          result += light.emission * reflectance * shadow_prd.attenuation;
        }
      }
    }

    for (int i = 0; i < point_lights.size(); i++) {
      PointLight light = point_lights[i];
      const float Ldist = length(light.position - hitpoint);
      const float3 L = normalize(light.position - hitpoint);
      float3 reflectance = ForwardDiffuse(L, -ray.direction, ffnormal, kd_val) +
                           ForwardGGX(L, -ray.direction, ffnormal, roughness, ks_val);
      if (reflectance.x > 0.f && reflectance.y > 0.f && reflectance.z > 0.f) {
        PerRayData_shadow shadow_prd;
        shadow_prd.attenuation = make_float3(1.0f);
        shadow_prd.inShadow = false;
        Ray shadow_ray =
            make_Ray(hitpoint, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist - scene_epsilon);
        rtTrace(top_shadower, shadow_ray, shadow_prd);

        if (!shadow_prd.inShadow) {
          result += reflectance * light.emission / Ldist / Ldist * shadow_prd.attenuation;
        }
      }
    }

    for (int i = 0; i < parallelogram_lights.size(); ++i) {
      ParallelogramLight light = parallelogram_lights[i];

      // sample a point on the light
      float r1 = rnd(current_prd.seed);
      float r2 = rnd(current_prd.seed);
      float3 light_position = light.corner + r1 * light.v1 + r2 * light.v2;
      const float Ldist = length(light_position - hitpoint);
      const float3 L = normalize(light_position - hitpoint);
      const float lnDl = dot(light.normal, L); // light normal dot light direction

      if (lnDl < 0.f) {
        float3 reflectance = ForwardDiffuse(L, -ray.direction, ffnormal, kd_val) +
                             ForwardGGX(L, -ray.direction, ffnormal, roughness, ks_val);
        if (reflectance.x > 0.f && reflectance.y > 0.f && reflectance.z > 0.f) {
          PerRayData_shadow shadow_prd;
          shadow_prd.attenuation = make_float3(1.0f);
          shadow_prd.inShadow = false;
          const float area = length(cross(light.v1, light.v2));
          Ray shadow_ray =
              make_Ray(hitpoint, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist - scene_epsilon);
          rtTrace(top_shadower, shadow_ray, shadow_prd);

          if (!shadow_prd.inShadow) {
            result +=  reflectance
                      * (-lnDl) * area // visible area
                      * light.emission / Ldist / Ldist * shadow_prd.attenuation;
          }
        }
      }
    }

  } else {
    // transmission
    current_prd.origin = hitpoint;
  }

  current_prd.radiance = result;
}

RT_PROGRAM void any_hit() {
  if (has_kd_map) {
    if (tex2D(kd_map, texcoord.x, texcoord.y).w < 0.1f) {
      rtIgnoreIntersection();
    }
  }
}

RT_PROGRAM void shadow_any_hit() {
  if (!use_shadow) {
    rtTerminateRay();
    return;
  }

  float3 kd_val = make_float3(kd);
  float alpha = kd.w;
  if (has_kd_map) {
    float4 texture_kd_val = tex2D(kd_map, texcoord.x, texcoord.y);
    kd_val = make_float3(texture_kd_val);
    alpha = texture_kd_val.w;
  }

  if (alpha == 1) {
    prd_shadow.inShadow = true;
    rtTerminateRay();
  } else {
    prd_shadow.attenuation *= (1 - alpha);
  }
}
