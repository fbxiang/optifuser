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

std::unique_ptr<Object> NewDebugObject() {
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

std::unique_ptr<Object> NewPlane() {
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

std::unique_ptr<Object> NewYZPlane() {
  std::vector<Vertex> vertices;
  vertices.push_back(Vertex(glm::vec3(0, 1, 1), glm::vec3(1, 0, 0),
                            glm::vec2(0, 1), glm::vec3(0, 0, -1),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, -1, 1), glm::vec3(1, 0, 0),
                            glm::vec2(0, 0), glm::vec3(0, 0, -1),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, -1, -1), glm::vec3(1, 0, 0),
                            glm::vec2(1, 0), glm::vec3(0, 0, -1),
                            glm::vec3(0, 1, 0)));
  vertices.push_back(Vertex(glm::vec3(0, 1, -1), glm::vec3(1, 0, 0),
                            glm::vec2(1, 1), glm::vec3(0, 0, -1),
                            glm::vec3(0, 1, 0)));
  std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
  static auto mesh = std::make_shared<TriangleMesh>(vertices, indices);
  auto obj = NewObject<Object>(mesh);
  obj->name = "YZPlane";
  return obj;
}

std::unique_ptr<Object> NewCube() {
  std::vector<Vertex> vertices = {
      Vertex(glm::vec3(-1.0, -1.0, 1.0)),  Vertex(glm::vec3(1.0, -1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, 1.0)),    Vertex(glm::vec3(-1.0, 1.0, 1.0)),
      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0))};
  std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1,
                                 7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4,
                                 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};
  static auto cubeMesh =
      std::make_shared<TriangleMesh>(vertices, indices, true);
  auto obj = NewObject<Object>(cubeMesh);
  obj->name = "cube";
  return obj;
}

std::unique_ptr<Object> NewSphere() {
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
  x->material.kd = {1, 0, 0};

  auto y = NewCube();
  y->scale = {0.01, 1, 0.01};
  y->position = {0, 1, 0};
  y->material.kd = {0, 1, 0};

  auto z = NewCube();
  z->scale = {0.01, 0.01, 1};
  z->position = {0, 0, 1};
  z->material.kd = {0, 0, 1};

  auto axes = NewObject<Object>();
  axes->addChild(std::move(x));
  axes->addChild(std::move(y));
  axes->addChild(std::move(z));

  // auto obj = NewObject<Object>(std::make_shared<LineMesh>(vertices,
  // indices)); obj->name = "Axes"; obj->material.kd = {1, 0, 0};
  return axes;
}

void Object::addChild(std::unique_ptr<Object> child) {
  assert(child->getScene() == nullptr);
  child->parent = this;
  children.push_back(std::move(child));
}

} // namespace Optifuser
