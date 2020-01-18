#include "optix_renderer.h"
#include "texture.h"

namespace Optifuser {
const std::string ACCEL = "Trbvh";

static std::string get_ptx_filename(std::string name) {
  return "ptx/cuda_compile_ptx_1_generated_" + name + ".cu.ptx";
}

OptixRenderer::OptixRenderer() {}

OptixRenderer::~OptixRenderer() { exit(); }

void OptixRenderer::exit() {
  if (initialized) {
    context->destroy();
    glDeleteTextures(1, &outputTex);
    outputTex = 0;
    glDeleteFramebuffers(1, &transferFbo);
    transferFbo = 0;
    if (screenVbo) {
      glDeleteBuffers(1, &screenVbo);
      screenVbo = 0;
    }
    initialized = false;
  }
}

void OptixRenderer::init(uint32_t w, uint32_t h) {
  if (initialized)
    return;

  width = w;
  height = h;
  initialized = true;

  context = optix::Context::create();
  context->setRayTypeCount(2);    // camera ray and shadow ray
  context->setEntryPointCount(1); // 1 program for rendering
  context->setStackSize(1800);

  context["scene_epsilon"]->setFloat(1e-3f);
  context["pathtrace_ray_type"]->setUint(0u);
  context["pathtrace_shadow_ray_type"]->setUint(1u);
  context["bad_color"]->setFloat(1000000.f, 0.f, 1000000.f);
  context["bg_color"]->setFloat(.34f, .53f, .92f);

  context["n_samples_sqrt"]->setUint(nSamplesSqrt);

  // create vbo for screen rendering
  glGenBuffers(1, &screenVbo);
  glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
  glBufferData(GL_ARRAY_BUFFER, 16 * width * height, 0, GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  optix::Buffer output_buffer = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, screenVbo);
  output_buffer->setFormat(RT_FORMAT_FLOAT4);
  output_buffer->setSize(width, height);
  context["output_buffer"]->set(output_buffer);

  std::string ptxFile = get_ptx_filename("pathtracer");
  context->setExceptionProgram(0, context->createProgramFromPTXFile(ptxFile, "exception"));
  context->setRayGenerationProgram(0, context->createProgramFromPTXFile(ptxFile, "camera"));

  glGenTextures(1, &outputTex);
  glBindTexture(GL_TEXTURE_2D, outputTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &transferFbo);
}

void OptixRenderer::initSceneGeometry(const Scene &scene) {
  sceneInitialized = true;
  optix::Group topGroup = context->createGroup();
  optix::Group shadowGroup = context->createGroup();
  topGroup->setAcceleration(context->createAcceleration(ACCEL));
  shadowGroup->setAcceleration(context->createAcceleration(ACCEL));

  for (const auto &obj : scene.getObjects()) {
    if (obj->getMesh() && obj->visible) {
      topGroup->addChild(getObjectTransform(obj.get()));
      shadowGroup->addChild(getObjectTransform(obj.get()));
    }
  }
  context["top_object"]->set(topGroup);
  context["top_shadower"]->set(shadowGroup);

  std::string ptxFile;
  if (backgroundMode == CUBEMAP) {
    printf("Using cubemap\n");
    ptxFile = get_ptx_filename("cubemap");

    auto sampler = context->createTextureSampler();
    sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
    sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
    sampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);

    auto buffer = context->createBuffer(RT_BUFFER_INPUT | RT_BUFFER_CUBEMAP,
                                        RT_FORMAT_UNSIGNED_BYTE4, cubemap.width, cubemap.width, 6);
    unsigned char *dst = static_cast<unsigned char *>(buffer->map());
    size_t image_size = cubemap.width * cubemap.width * 4;
    memcpy(dst, cubemap.front.data(), image_size);
    memcpy(dst + image_size, cubemap.back.data(), image_size);
    memcpy(dst + 2 * image_size, cubemap.top.data(), image_size);
    memcpy(dst + 3 * image_size, cubemap.bottom.data(), image_size);
    memcpy(dst + 4 * image_size, cubemap.left.data(), image_size);
    memcpy(dst + 5 * image_size, cubemap.right.data(), image_size);

    buffer->unmap();
    sampler->setBuffer(buffer);

    context["envmapId"]->setInt(sampler->getId());
    context->setMissProgram(0, context->createProgramFromPTXFile(ptxFile, "miss"));
  } else if (backgroundMode == HDRMAP) {
    printf("Using HDR map");
    ptxFile = get_ptx_filename("hdrmap");

    auto sampler = context->createTextureSampler();
    sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
    sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
    auto buffer =
        context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, hdrmap.height * 2, hdrmap.height);
    float *dst = static_cast<float *>(buffer->map());
    size_t image_size = 2 * hdrmap.height * hdrmap.height * sizeof(float) * 4;
    memcpy(dst, hdrmap.texture.data(), image_size);
    buffer->unmap();
    sampler->setBuffer(buffer);

    context["envmap"]->setTextureSampler(sampler);
    context->setMissProgram(0, context->createProgramFromPTXFile(ptxFile, "miss"));
  } else if (backgroundMode == PROCEDUAL_SKY) {
    ptxFile = get_ptx_filename("procedural_sky");
    context->setMissProgram(0, context->createProgramFromPTXFile(ptxFile, "miss"));
  } else {
    ptxFile = get_ptx_filename("constantbg");
    context["bg_color"]->setFloat(0, 0, 0);
    context->setMissProgram(0, context->createProgramFromPTXFile(ptxFile, "miss"));
  }

  initSceneLights(scene);
}

void OptixRenderer::initSceneLights(const Scene &scene) {
  optix::Buffer lBuffer = context->createBuffer(RT_BUFFER_INPUT);
  lBuffer->setFormat(RT_FORMAT_USER);
  lBuffer->setElementSize(sizeof(DirectionalLight));
  lBuffer->setSize(scene.getDirectionalLights().size());
  DirectionalLight *dls = (DirectionalLight *)lBuffer->map();
  for (size_t i = 0; i < scene.getDirectionalLights().size(); i++) {
    dls[i] = scene.getDirectionalLights()[i];
  }
  lBuffer->unmap();
  context["directional_lights"]->setBuffer(lBuffer);

  lBuffer = context->createBuffer(RT_BUFFER_INPUT);
  lBuffer->setFormat(RT_FORMAT_USER);
  lBuffer->setElementSize(sizeof(PointLight));
  lBuffer->setSize(scene.getPointLights().size());
  PointLight *pls = (PointLight *)lBuffer->map();
  for (size_t i = 0; i < scene.getPointLights().size(); i++) {
    pls[i] = scene.getPointLights()[i];
  }
  lBuffer->unmap();
  context["point_lights"]->setBuffer(lBuffer);

  lBuffer = context->createBuffer(RT_BUFFER_INPUT);
  lBuffer->setFormat(RT_FORMAT_USER);
  lBuffer->setElementSize(sizeof(ParallelogramLight));
  lBuffer->setSize(scene.getParallelogramLights().size());
  ParallelogramLight *als = (ParallelogramLight *)lBuffer->map();
  for (size_t i = 0; i < scene.getParallelogramLights().size(); i++) {
    als[i] = scene.getParallelogramLights()[i];
  }
  lBuffer->unmap();
  context["parallelogram_lights"]->setBuffer(lBuffer);
}

optix::Transform OptixRenderer::getObjectTransform(const Object *obj) {
  if (!obj) {
    return 0;
  }
  auto p = _object_transform.find(obj);

  optix::Transform transform = 0;
  if (p == _object_transform.end()) {
    if (!obj->getMesh())
      return 0;

    optix::GeometryInstance gio = context->createGeometryInstance();
    if (std::shared_ptr<TriangleMesh> mesh =
            std::dynamic_pointer_cast<TriangleMesh>(obj->getMesh())) {
      gio->setGeometry(getMeshGeometry(mesh.get()));
    } else if (std::shared_ptr<DynamicMesh> mesh =
                   std::dynamic_pointer_cast<DynamicMesh>(obj->getMesh())) {
      gio->setGeometry(getMeshGeometry(mesh.get()));
    } else {
      printf("ERROR\n");
    }

    // create material
    gio->setMaterialCount(1);
    optix::Material mat = context->createMaterial();
    if (!_material_closest_hit) {
      std::string ptxFile = get_ptx_filename("material");

      _material_closest_hit = context->createProgramFromPTXFile(ptxFile, "closest_hit");
      _material_shadow_any_hit = context->createProgramFromPTXFile(ptxFile, "shadow_any_hit");
      _material_any_hit = context->createProgramFromPTXFile(ptxFile, "any_hit");
    }
    if (!_material_fluid_closest_hit) {
      std::string ptxFile = get_ptx_filename("material_transparent");

      _material_fluid_closest_hit = context->createProgramFromPTXFile(ptxFile, "closest_hit");
      _material_fluid_shadow_any_hit =
          context->createProgramFromPTXFile(ptxFile, "shadow_any_hit");
      _material_fluid_any_hit = context->createProgramFromPTXFile(ptxFile, "any_hit");
    }
    if (!_material_mirror_closest_hit) {
      std::string ptxFile = get_ptx_filename("material_mirror");

      _material_mirror_closest_hit = context->createProgramFromPTXFile(ptxFile, "closest_hit");
      _material_mirror_shadow_any_hit =
          context->createProgramFromPTXFile(ptxFile, "shadow_any_hit");
      _material_mirror_any_hit = context->createProgramFromPTXFile(ptxFile, "any_hit");
    }

    if (obj->material.type == Optifuser::Material::Type::TRANSPARENT) {
      mat->setClosestHitProgram(0, _material_fluid_closest_hit);
      mat->setAnyHitProgram(0, _material_fluid_any_hit);
      mat->setAnyHitProgram(1, _material_fluid_shadow_any_hit);
      mat["tint"]->setFloat(obj->material.kd.x, obj->material.kd.y, obj->material.kd.z);
    } else if (obj->material.type == Optifuser::Material::Type::MIRROR) {
      mat->setClosestHitProgram(0, _material_mirror_closest_hit);
      mat->setAnyHitProgram(0, _material_mirror_any_hit);
      mat->setAnyHitProgram(1, _material_mirror_shadow_any_hit);
    } else {
      mat->setClosestHitProgram(0, _material_closest_hit);
      mat->setAnyHitProgram(0, _material_any_hit);
      mat->setAnyHitProgram(1, _material_shadow_any_hit);
      mat["kd"]->setFloat(obj->material.kd.x, obj->material.kd.y, obj->material.kd.z,
                          obj->material.kd.w);
      if (obj->material.kd_map->getId()) {
        mat["has_kd_map"]->setInt(1);
        mat["kd_map"]->setTextureSampler(getTextureSampler(obj->material.kd_map.get()));
      } else {
        mat["has_kd_map"]->setInt(0);
        mat["kd_map"]->setTextureSampler(getEmptySampler());
      }
    }
    gio->setMaterial(0, mat);

    optix::GeometryGroup gg = context->createGeometryGroup();
    gg->setChildCount(1);
    gg->setChild(0, gio);

    gg->setAcceleration(getObjectAccel(obj));

    transform = context->createTransform();
    glm::mat4 modelMat = obj->getModelMat();

    float m[16];
    for (int i = 0; i < 16; i++) {
      m[i] = modelMat[i / 4][i % 4];
    }

    transform->setMatrix(true, m, nullptr);
    transform->setChild(gg);

    _object_transform[obj] = transform;
  } else {
    transform = p->second;
  }
  return transform;
}

optix::Geometry OptixRenderer::getMeshGeometry(const TriangleMesh *mesh) {
  if (!mesh) {
    return 0;
  }

  auto p = _mesh_geometry.find(mesh);
  optix::Geometry g = 0;
  if (p == _mesh_geometry.end()) {
    optix::Buffer vertices = context->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getVBO());
    vertices->setFormat(RT_FORMAT_USER);
    vertices->setElementSize(sizeof(float) * 14);
    vertices->setSize(mesh->getVertices().size());

    optix::Buffer indices = context->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getEBO());
    indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
    indices->setSize(mesh->getIndices().size() / 3);

    g = context->createGeometry();
    g["vertex_buffer"]->setBuffer(vertices);
    g["index_buffer"]->setBuffer(indices);
    g->setPrimitiveCount((uint32_t)mesh->size());

    if (!_mesh_intersect) {
      std::string ptxFile = get_ptx_filename("triangle_mesh");
      _mesh_intersect = context->createProgramFromPTXFile(ptxFile, "mesh_intersect");
      _mesh_bounds = context->createProgramFromPTXFile(ptxFile, "mesh_bounds");
    }

    g->setBoundingBoxProgram(_mesh_bounds);
    g->setIntersectionProgram(_mesh_intersect);

    _mesh_geometry[mesh] = g;
  } else {
    g = p->second;
  }

  return g;
}

optix::Geometry OptixRenderer::getMeshGeometry(const DynamicMesh *mesh) {
  if (!mesh) {
    return 0;
  }

  auto p = _dmesh_geometry.find(mesh);
  optix::Geometry g = 0;
  if (p == _dmesh_geometry.end()) {
    optix::Buffer vertices = context->createBufferFromGLBO(RT_BUFFER_INPUT, mesh->getVBO());
    vertices->setFormat(RT_FORMAT_USER);
    vertices->setElementSize(sizeof(float) * 6);
    vertices->setSize(mesh->getMaxVertexCount());

    // optix::Buffer indices = context->createBufferFromGLBO(RT_BUFFER_INPUT,
    // mesh->getEBO()); indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
    // indices->setSize(mesh->getIndices().size() / 3);

    g = context->createGeometry();
    g["vertex_buffer"]->setBuffer(vertices);
    // g["index_buffer"]->setBuffer(indices);
    g->setPrimitiveCount(mesh->getVertexCount() / 3);

    if (!_dmesh_intersect) {
      std::string ptxFile = get_ptx_filename("dynamic_mesh");
      _dmesh_intersect = context->createProgramFromPTXFile(ptxFile, "mesh_intersect");
      _dmesh_bounds = context->createProgramFromPTXFile(ptxFile, "mesh_bounds");
    }

    g->setBoundingBoxProgram(_dmesh_bounds);
    g->setIntersectionProgram(_dmesh_intersect);

    _dmesh_geometry[mesh] = g;
  } else {
    g = p->second;
  }

  return g;
}

optix::Acceleration OptixRenderer::getObjectAccel(const Object *obj) {
  if (!obj) {
    return 0;
  }

  auto p = _object_accel.find(obj);
  optix::Acceleration accel;
  if (p == _object_accel.end()) {
    accel = context->createAcceleration(ACCEL);
    _object_accel[obj] = accel;
  } else {
    accel = p->second;
  }
  return accel;
}

optix::TextureSampler OptixRenderer::getTextureSampler(const Texture *tex) {
  if (!tex) {
    return 0;
  }

  if (tex->getId() == 0) {
    return 0;
  }

  auto p = _texture_sampler.find(tex);
  optix::TextureSampler sampler = 0;
  if (p == _texture_sampler.end()) {
    sampler = context->createTextureSamplerFromGLImage(tex->getId(), RT_TARGET_GL_TEXTURE_2D);
    _texture_sampler[tex] = sampler;
  } else {
    sampler = p->second;
  }

  return sampler;
}

optix::TextureSampler OptixRenderer::getEmptySampler() {
  if (!_empty_sampler) {
    _empty_sampler = context->createTextureSampler();
    optix::Buffer buffer =
        context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, 1u, 1u);
    _empty_sampler->setBuffer(0u, 0u, buffer);
  }
  return _empty_sampler;
}

void OptixRenderer::setCubemap(std::string const &front, std::string const &back,
                               std::string const &top, std::string const &bottom,
                               std::string const &left, std::string const &right) {
  backgroundMode = CUBEMAP;

  {
    // front
    auto [data, width, height, nChannels] = load_image(front);
    assert(width == height && nChannels == 4);
    cubemap.front = std::move(data);
    cubemap.width = width;
  }
  {
    // back
    auto [data, width, height, nChannels] = load_image(back);
    assert(cubemap.width == width && cubemap.width == height && nChannels == 4);
    cubemap.back = std::move(data);
  }
  {
    // top
    auto [data, width, height, nChannels] = load_image(top);
    assert(cubemap.width == width && cubemap.width == height && nChannels == 4);
    cubemap.top = std::move(data);
  }
  {
    // bottom
    auto [data, width, height, nChannels] = load_image(bottom);
    assert(cubemap.width == width && cubemap.width == height && nChannels == 4);
    cubemap.bottom = std::move(data);
  }
  {
    // left
    auto [data, width, height, nChannels] = load_image(left);
    assert(cubemap.width == width && cubemap.width == height && nChannels == 4);
    cubemap.left = std::move(data);
  }
  {
    // right
    auto [data, width, height, nChannels] = load_image(right);
    assert(cubemap.width == width && cubemap.width == height && nChannels == 4);
    cubemap.right = std::move(data);
  }
}

void OptixRenderer::setHdrmap(std::string const &map) {
  backgroundMode = HDRMAP;

  auto [data, width, height, nChannels] = load_hdr(map);
  assert(width == 2 * height);
  hdrmap.height = height;
  hdrmap.texture = std::move(data);

  cubemap = {};
}

void OptixRenderer::renderScene(const Scene &scene, const CameraSpec &camera) {
  if (iterations >= max_iterations) {
    return;
  }

  // initialize scene
  if (!sceneInitialized) {
    initSceneGeometry(scene);
  }

  // update camera
  glm::vec3 u = camera.getRotation() * glm::vec3(1, 0, 0);
  glm::vec3 v = camera.getRotation() * glm::vec3(0, 1, 0);
  glm::vec3 w = camera.getRotation() * glm::vec3(0, 0, 1);

  optix::float3 W = -optix::make_float3(w.x, w.y, w.z);
  float vlen = tanf(0.5f * camera.fovy);
  optix::float3 V = vlen * optix::make_float3(v.x, v.y, v.z);
  float ulen = vlen * camera.aspect;
  optix::float3 U = ulen * optix::make_float3(u.x, u.y, u.z);

  context["eye"]->setFloat(camera.position.x, camera.position.y, camera.position.z);
  context["U"]->setFloat(U);
  context["V"]->setFloat(V);
  context["W"]->setFloat(W);
  context["near"]->setFloat(camera.near);
  context["far"]->setFloat(camera.far);
  context["iterations"]->setUint(iterations);
  context["n_rays"]->setUint(numRays);
  context["use_shadow"]->setInt(useShadow);

  // update dynamic meshes
  for (auto mesh : _dmesh_geometry) {
    int count = mesh.first->getVertexCount() / 3;
    printf("Updating mesh to %d\n", count);
    mesh.second->setPrimitiveCount(count);
    mesh.second->markDirty();
  }
  for (auto &obj : scene.getObjects()) {
    auto transform = getObjectTransform(obj.get());
    if (transform) {
      transform->getChild<optix::GeometryGroup>()->getAcceleration()->markDirty();
    }
  }
  context["top_object"]->getGroup()->getAcceleration()->markDirty();

  // ray trace
  context->launch(0, width, height);
  iterations++;
}

void OptixRenderer::display() {
  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, outputTex);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context["output_buffer"]->getBuffer()->getGLBOId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  // read from segmentation color
  glBindFramebuffer(GL_READ_FRAMEBUFFER, transferFbo);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTex, 0);

  // draw to given fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

std::vector<float> OptixRenderer::getResult() {
  glClear(GL_COLOR_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, outputTex);
  glEnable(GL_TEXTURE_2D);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context["output_buffer"]->getBuffer()->getGLBOId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  return getRGBAFloat32Texture(outputTex, width, height);
}

void OptixRenderer::renderSceneToFile(const Scene &scene, const CameraSpec &cam,
                                      std::string filename) {
  if (iterations >= max_iterations)
    return;

  // initialize scene
  if (!sceneInitialized) {
    printf("Initializing scene gemoetry...\n");
    initSceneGeometry(scene);
  }

  // update camera
  glm::vec3 u = cam.getRotation() * glm::vec3(1, 0, 0);
  glm::vec3 v = cam.getRotation() * glm::vec3(0, 1, 0);
  glm::vec3 w = cam.getRotation() * glm::vec3(0, 0, 1);

  optix::float3 W = -optix::make_float3(w.x, w.y, w.z);
  float vlen = tanf(0.5f * cam.fovy);
  optix::float3 V = vlen * optix::make_float3(v.x, v.y, v.z);
  float ulen = vlen * cam.aspect;
  optix::float3 U = ulen * optix::make_float3(u.x, u.y, u.z);

  context["eye"]->setFloat(cam.position.x, cam.position.y, cam.position.z);
  context["U"]->setFloat(U);
  context["V"]->setFloat(V);
  context["W"]->setFloat(W);
  context["near"]->setFloat(cam.near);
  context["far"]->setFloat(cam.far);
  context["iterations"]->setUint(iterations);
  context["n_rays"]->setUint(numRays);
  context["use_shadow"]->setInt(useShadow);

  // ray trace
  context->launch(0, width, height);

  glClear(GL_COLOR_BUFFER_BIT);

  // display
  // sutil::displayBufferGL(context["output_buffer"]->getBuffer());

  glBindTexture(GL_TEXTURE_2D, outputTex);
  glEnable(GL_TEXTURE_2D);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context["output_buffer"]->getBuffer()->getGLBOId());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  writeToFile(outputTex, width, height, filename);

  iterations++;
}

// void OptixRenderer::renderCurrentToFile(std::string filename) {
//   glBindTexture(GL_TEXTURE_2D, outputTex);
//   glBindBuffer(GL_PIXEL_UNPACK_BUFFER,
//                context["output_buffer"]->getBuffer()->getGLBOId());
//   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
//                GL_FLOAT, 0);
//   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
//   writeToFile(outputTex, width, height, filename);
// }

} // namespace Optifuser
