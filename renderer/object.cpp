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

void Object::setScene(const std::shared_ptr<Scene> inScene) { scene = inScene; }

std::shared_ptr<Scene> Object::getScene() const {
  if (!scene.expired())
    return scene.lock();
  return std::shared_ptr<Scene>();
}

std::shared_ptr<AbstractMeshBase> Object::getMesh() const { return mesh; }

std::shared_ptr<Object> NewNoisePlane(unsigned int res) {

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
      vertices.push_back(Vertex(glm::vec3(x, noise(x, z), z), glm::vec3(0),
                                glm::vec2(i * d, j * d)));
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

std::shared_ptr<Object> NewDebugObject() {
  std::vector<Vertex> vertices;
  vertices.push_back(
      Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1)));
  vertices.push_back(
      Vertex(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)));
  vertices.push_back(
      Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0)));
  vertices.push_back(
      Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1)));

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

  auto obj =
      NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices));
  obj->name = "Debug";
  return obj;
}

std::shared_ptr<Object> NewPlane() {
  std::vector<Vertex> vertices;
  vertices.push_back(Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1),
                            glm::vec2(0, 1), glm::vec3(1, 0, 0),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1),
                            glm::vec2(0, 0), glm::vec3(1, 0, 0),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1),
                            glm::vec2(1, 0), glm::vec3(1, 0, 0),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1),
                            glm::vec2(1, 1), glm::vec3(1, 0, 0),
                            glm::vec3(0, 1, 0)));

  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
  auto obj =
      NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices));
  obj->name = "Plane";
  return obj;
}

std::shared_ptr<Object> NewCube() {
  auto obj = NewObject<Object>(NewCubeMesh());
  obj->name = "cube";
  return obj;
}

std::shared_ptr<Object> NewSphere() {
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  int stacks = 5;
  int slices = 5;
  float radius = 1.f;

  for (int i = 0; i <= stacks; ++i) {

    GLfloat V = i / (float)stacks;
    GLfloat phi = V * glm::pi<float>();

    // Loop Through Slices
    for (int j = 0; j <= slices; ++j) {

      GLfloat U = j / (float)slices;
      GLfloat theta = U * (glm::pi<float>() * 2);

      // Calc The Vertex Positions
      GLfloat x = cosf(theta) * sinf(phi);
      GLfloat y = cosf(phi);
      GLfloat z = sinf(theta) * sinf(phi);

      // Push Back Vertex Data
      vertices.push_back(Vertex({x * radius, y * radius, z * radius}));
    }
  }

  // Calc The Index Positions
  for (int i = 0; i < slices * stacks + slices; ++i) {
    indices.push_back(i);
    indices.push_back(i + slices + 1);
    indices.push_back(i + slices);

    indices.push_back(i + slices + 1);
    indices.push_back(i);
    indices.push_back(i + 1);
  }

  auto obj = NewObject<Object>(
      std::make_shared<TriangleMesh>(vertices, indices, true));
  obj->name = "Sphere";
  return obj;
}

std::shared_ptr<Object> NewLine() {
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

std::shared_ptr<Object> NewLineCube() {
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

std::shared_ptr<Object> NewEnvironmentCube() {
  std::vector<Vertex> vertices(8);
  vertices[0].position = {1.f, 1.f, 1.f};
  vertices[1].position = {-1.f, 1.f, 1.f};
  vertices[2].position = {1.f, -1.f, 1.f};
  vertices[3].position = {-1.f, -1.f, 1.f};
  vertices[4].position = {1.f, 1.f, -1.f};
  vertices[5].position = {-1.f, 1.f, -1.f};
  vertices[6].position = {1.f, -1.f, -1.f};
  vertices[7].position = {-1.f, -1.f, -1.f};

  for (auto &v : vertices) {
    v.position.x *= 500;
    v.position.y *= 500;
    v.position.z *= 500;
  }

  std::vector<unsigned int> indices = {0, 1, 2, 1, 3, 2, 4, 0, 6, 0, 2, 6,
                                       5, 4, 7, 4, 6, 7, 1, 5, 3, 5, 7, 3,
                                       4, 5, 0, 5, 1, 0, 3, 7, 6, 2, 3, 6};

  return NewObject<Object>(std::make_shared<TriangleMesh>(vertices, indices));
}
} // namespace Optifuser
