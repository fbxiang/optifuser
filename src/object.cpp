#include "object.h"
namespace Optifuser {
glm::mat4 Object::getModelMat() const {
  glm::mat4 t = glm::toMat4(rotation);
  t[0] *= scale.x;
  t[1] *= scale.y;
  t[2] *= scale.z;
  t[3][0] = position.x;
  t[3][1] = position.y;
  t[3][2] = position.z;

  return t;
}

void Object::setScene(Scene *inScene) { scene = inScene; }

Scene *Object::getScene() const { return scene; }

std::shared_ptr<AbstractMeshBase> Object::getMesh() const { return mesh; }

void Object::addChild(std::unique_ptr<Object> child) {
  assert(child->getScene() == nullptr);
  child->parent = this;
  children.push_back(std::move(child));
}

std::unique_ptr<Object> NewNoisePlane(unsigned int res) {

  PerlinNoise noise;
  noise.addNoise(0.1, glm::vec2(0), 2, 0);
  noise.addNoise(0.01, glm::vec2(3, 4), 20, 7);
  noise.addNoise(0.05, glm::vec2(1, -1), 4, 2);

  std::vector<Vertex> vertices;
  float d = 1.f / res;
  for (uint32_t i = 0; i < res + 1; i++) {
    for (uint32_t j = 0; j < res + 1; j++) {
      float x = -0.5 + i * d;
      float z = -0.5 + j * d;
      vertices.push_back(
          Vertex(glm::vec3(x, noise(x, z), z), glm::vec3(0), glm::vec2(i * d, j * d)));
    }
  }

  std::vector<unsigned int> indices;
  for (unsigned int i = 0; i < res; i++) {
    for (unsigned int j = 0; j < res; j++) {
      unsigned int v1 = i * (res + 1) + j;
      unsigned int v2 = i * (res + 1) + j + 1;
      unsigned int v3 = (i + 1) * (res + 1) + j;
      unsigned int v4 = (i + 1) * (res + 1) + j + 1;
      indices.push_back(v1);
      indices.push_back(v2);
      indices.push_back(v3);
      indices.push_back(v3);
      indices.push_back(v2);
      indices.push_back(v4);
    }
  }

  auto mesh = std::make_shared<TriangleMesh>(vertices, indices);
  auto obj = NewObject<Object>(mesh);
  return obj;
}

std::unique_ptr<Object> NewDebugObject() {
  std::vector<Vertex> vertices;
  vertices.push_back(Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1)));
  vertices.push_back(Vertex(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
  vertices.push_back(Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0)));
  vertices.push_back(Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1)));

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  auto obj = NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices));
  obj->name = "Debug";
  return obj;
}

std::unique_ptr<Object> NewPlane() {
  std::vector<Vertex> vertices;
  vertices.push_back(Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1),
                            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0),
                            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0),
                            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1),
                            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)));

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
  auto obj = NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices));
  obj->name = "Plane";
  return obj;
}

std::unique_ptr<Object> NewYZPlane() {
  std::vector<Vertex> vertices;
  vertices.push_back(Vertex(glm::vec3(0, 1, 1), glm::vec3(1, 0, 0), glm::vec2(0, 1),
                            glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, -1, 1), glm::vec3(1, 0, 0), glm::vec2(0, 0),
                            glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, -1, -1), glm::vec3(1, 0, 0), glm::vec2(1, 0),
                            glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, 1, -1), glm::vec3(1, 0, 0), glm::vec2(1, 1),
                            glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)));
  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
  static auto mesh = std::make_shared<TriangleMesh>(vertices, indices);
  auto obj = NewObject<Object>(mesh);
  obj->name = "YZPlane";
  return obj;
}

std::unique_ptr<Object> NewFlatCube() {
  std::vector<Vertex> vertices = {
      Vertex(glm::vec3(-1.0, -1.0, 1.0)),  Vertex(glm::vec3(1.0, -1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, 1.0)),    Vertex(glm::vec3(-1.0, 1.0, 1.0)),

      Vertex(glm::vec3(1.0, -1.0, 1.0)),   Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(1.0, 1.0, 1.0)),

      Vertex(glm::vec3(-1.0, 1.0, -1.0)),  Vertex(glm::vec3(1.0, 1.0, -1.0)),
      Vertex(glm::vec3(1.0, -1.0, -1.0)),  Vertex(glm::vec3(-1.0, -1.0, -1.0)),

      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(-1.0, -1.0, 1.0)),
      Vertex(glm::vec3(-1.0, 1.0, 1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0)),

      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, -1.0, 1.0)),   Vertex(glm::vec3(-1.0, -1.0, 1.0)),

      Vertex(glm::vec3(-1.0, 1.0, 1.0)),   Vertex(glm::vec3(1.0, 1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0))};
  std::vector<GLuint> indices = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,
                                 8,  9,  10, 10, 11, 8,  12, 13, 14, 14, 15, 12,
                                 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

  static auto cubeMesh = std::make_shared<TriangleMesh>(vertices, indices, true);
  auto obj = NewObject<Object>(cubeMesh);
  obj->name = "FlatCube";
  return obj;
}

std::unique_ptr<Object> NewCube() {
  std::vector<Vertex> vertices = {
      Vertex(glm::vec3(-1.0, -1.0, 1.0)),  Vertex(glm::vec3(1.0, -1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, 1.0)),    Vertex(glm::vec3(-1.0, 1.0, 1.0)),
      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0))};
  std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                                 4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};
  static auto cubeMesh = std::make_shared<TriangleMesh>(vertices, indices, true);
  auto obj = NewObject<Object>(cubeMesh);
  obj->name = "cube";
  return obj;
}

std::unique_ptr<Object> NewSphere() {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  int stacks = 20;
  int slices = 20;
  float radius = 1.f;

  for (uint32_t i = 1; i < stacks; ++i) {
    float phi = glm::pi<float>() / stacks * i - glm::pi<float>() / 2;
    for (uint32_t j = 0; j < slices; ++j) {
      float theta = glm::pi<float>() * 2 / slices * j;
      float x = sinf(phi) * radius;
      float y = cosf(theta) * cosf(phi) * radius;
      float z = sinf(theta) * cosf(phi) * radius;
      vertices.push_back({{x, y, z}});
    }
  }

  for (uint32_t i = 0; i < (stacks - 2) * slices; ++i) {
    uint32_t right = (i + 1) % slices + i / slices * slices;
    uint32_t up = i + slices;
    uint32_t rightUp = right + slices;

    indices.push_back(i);
    indices.push_back(rightUp);
    indices.push_back(up);

    indices.push_back(i);
    indices.push_back(right);
    indices.push_back(rightUp);
  }

  vertices.push_back({{-radius, 0, 0}});
  vertices.push_back({{radius, 0, 0}});

  for (uint32_t i = 0; i < slices; ++i) {
    uint32_t right = (i + 1) % slices + i / slices * slices;
    indices.push_back(vertices.size() - 2);
    indices.push_back(right);
    indices.push_back(i);
  }
  for (uint32_t i = (stacks - 2) * slices; i < (stacks - 1) * slices; ++i) {
    uint32_t right = (i + 1) % slices + i / slices * slices;
    indices.push_back(vertices.size() - 1);
    indices.push_back(i);
    indices.push_back(right);
  }

  auto obj = NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices, true));
  obj->name = "Sphere";
  return obj;
}

std::unique_ptr<Object> NewLine() {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  vertices.push_back(Vertex({0, 0, 0}));
  vertices.push_back(Vertex({1, 0, 0}));
  indices.push_back(0);
  indices.push_back(1);

  auto obj = NewObject<Object>(std::make_shared<LineMesh>(vertices, indices));
  obj->name = "Line";
  return obj;
}

std::unique_ptr<Object> NewLineCube() {
  std::vector<Vertex> vertices = {
      Vertex(glm::vec3(-1.0, -1.0, 1.0)),  Vertex(glm::vec3(1.0, -1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, 1.0)),    Vertex(glm::vec3(-1.0, 1.0, 1.0)),
      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0))};

  std::vector<GLuint> indices = {0, 1, 1, 2, 2, 3, 0, 3, 4, 5, 5, 6,
                                 6, 7, 4, 7, 0, 4, 1, 5, 2, 6, 3, 7};
  auto obj = NewObject<Object>(std::make_shared<LineMesh>(vertices, indices));
  obj->name = "Line Cube";
  return obj;
}

std::unique_ptr<Object> NewMeshGrid() {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  int c = 0;
  for (int i = -5; i <= 5; ++i) {
    vertices.push_back(glm::vec3(i, 0, -5));
    indices.push_back(c++);
    vertices.push_back(glm::vec3(i, 0, 5));
    indices.push_back(c++);
  }
  for (int i = -5; i <= 5; ++i) {
    vertices.push_back(glm::vec3(-5, 0, i));
    indices.push_back(c++);
    vertices.push_back(glm::vec3(5, 0, i));
    indices.push_back(c++);
  }
  auto obj = NewObject<Object>(std::make_shared<LineMesh>(vertices, indices));
  return obj;
}

std::unique_ptr<Object> NewAxes() {
  auto x = NewCube();
  x->scale = {1, 0.01, 0.01};
  x->position = {1, 0, 0};
  x->material.kd = {1, 0, 0, 1};

  auto y = NewCube();
  y->scale = {0.01, 1, 0.01};
  y->position = {0, 1, 0};
  y->material.kd = {0, 1, 0, 1};

  auto z = NewCube();
  z->scale = {0.01, 0.01, 1};
  z->position = {0, 0, 1};
  z->material.kd = {0, 0, 1, 1};

  auto axes = NewObject<Object>();
  axes->addChild(std::move(x));
  axes->addChild(std::move(y));
  axes->addChild(std::move(z));

  // auto obj = NewObject<Object>(std::make_shared<LineMesh>(vertices,
  // indices)); obj->name = "Axes"; obj->material.kd = {1, 0, 0};
  return axes;
}

std::unique_ptr<Object> NewCapsule(float halfHeight, float radius) {
  std::vector<Vertex> vertices1;
  std::vector<Vertex> vertices2;
  std::vector<GLuint> indices1;
  std::vector<GLuint> indices2;

  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  uint32_t stacks = 10;
  uint32_t slices = 20;

  for (uint32_t i = 0; i < stacks; ++i) {
    float phi = glm::pi<float>() / 2 / stacks * i;
    for (uint32_t j = 0; j < slices; ++j) {
      float theta = glm::pi<float>() * 2 / slices * j;
      float x = sinf(phi) * radius;
      float y = cosf(theta) * cosf(phi) * radius;
      float z = sinf(theta) * cosf(phi) * radius;
      vertices1.push_back({{x + halfHeight, y, z}});
      vertices2.push_back({{-x - halfHeight, y, z}});
    }
  }
  vertices1.push_back({{radius + halfHeight, 0, 0}});
  vertices2.push_back({{-radius - halfHeight, 0, 0}});

  for (uint32_t i = 0; i < (stacks - 1) * slices; ++i) {
    uint32_t right = (i + 1) % slices + i / slices * slices;
    uint32_t up = i + slices;
    uint32_t rightUp = right + slices;

    indices1.push_back(i);
    indices1.push_back(rightUp);
    indices1.push_back(up);

    indices1.push_back(i);
    indices1.push_back(right);
    indices1.push_back(rightUp);

    indices2.push_back(i);
    indices2.push_back(up);
    indices2.push_back(rightUp);

    indices2.push_back(i);
    indices2.push_back(rightUp);
    indices2.push_back(right);
  }

  for (uint32_t i = 0; i < slices; ++i) {
    uint32_t curr = (stacks - 1) * slices + i;
    uint32_t right = (curr + 1) % slices + curr / slices * slices;
    uint32_t up = vertices1.size() - 1;

    indices1.push_back(curr);
    indices1.push_back(right);
    indices1.push_back(up);

    indices2.push_back(curr);
    indices2.push_back(up);
    indices2.push_back(right);
  }

  vertices = vertices1;
  vertices.insert(vertices.end(), vertices2.begin(), vertices2.end());

  indices = indices1;
  for (auto i : indices2) {
    indices.push_back(vertices1.size() + i);
  }

  for (uint32_t i = 0; i < slices; ++i) {
    uint32_t right = (i + 1) % slices;
    uint32_t up = i + vertices1.size();
    uint32_t rightUp = right + vertices1.size();

    indices.push_back(i);
    indices.push_back(up);
    indices.push_back(rightUp);

    indices.push_back(i);
    indices.push_back(rightUp);
    indices.push_back(right);
  }

  auto obj = NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices, true));
  obj->name = "Capsule";
  return obj;
}


} // namespace Optifuser
