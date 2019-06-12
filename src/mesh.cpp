#include "mesh.h"

namespace Optifuser {
MeshBase::MeshBase() : vao(0), vbo(0), ebo(0) {}

MeshBase::~MeshBase() {
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (ebo)
    glDeleteBuffers(1, &ebo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}

MeshBase::MeshBase(const std::vector<Vertex> &inVertices,
                   const std::vector<GLuint> &inIndices) {
  vertices = inVertices;
  indices = inIndices;

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(3 * sizeof(float)));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(6 * sizeof(float)));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(8 * sizeof(float)));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(11 * sizeof(float)));

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               &indices[0], GL_STATIC_DRAW);
}

GLuint MeshBase::getVAO() const { return vao; }
GLuint MeshBase::getVBO() const { return vbo; }
GLuint MeshBase::getEBO() const { return ebo; }

const std::vector<Vertex> &MeshBase::getVertices() const { return vertices; }

const std::vector<GLuint> &MeshBase::getIndices() const { return indices; }

TriangleMesh::TriangleMesh(const std::vector<Vertex> &inVertices,
                           const std::vector<GLuint> &inIndices,
                           bool recalcNormal) {
  vertices = inVertices;
  indices = inIndices;

  if (recalcNormal)
    recalculateNormals();

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(3 * sizeof(float)));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(6 * sizeof(float)));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(8 * sizeof(float)));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(11 * sizeof(float)));

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               &indices[0], GL_STATIC_DRAW);
}

void TriangleMesh::recalculateNormals() {
  for (auto &v : vertices) {
    v.normal = glm::vec3(0);
  }
  for (size_t i = 0; i < indices.size(); i += 3) {
    unsigned int i1 = indices[i];
    unsigned int i2 = indices[i + 1];
    unsigned int i3 = indices[i + 2];
    Vertex &v1 = vertices[i1];
    Vertex &v2 = vertices[i2];
    Vertex &v3 = vertices[i3];

    glm::vec3 normal = glm::normalize(
        glm::cross(v2.position - v1.position, v3.position - v1.position));
    if (std::isnan(normal.x)) {
      continue;
    }
    v1.normal += normal;
    v2.normal += normal;
    v3.normal += normal;
  }

  for (size_t i = 0; i < vertices.size(); i++) {
    if (vertices[i].normal == glm::vec3(0))
      continue;
    vertices[i].normal = glm::normalize(vertices[i].normal);
  }
}

void TriangleMesh::draw() const {
  glBindVertexArray(getVAO());
  glDrawElements(GL_TRIANGLES, getIndices().size(), GL_UNSIGNED_INT, 0);
}

std::shared_ptr<TriangleMesh> NewCubeMesh() {
  std::vector<Vertex> vertices = {
      Vertex(glm::vec3(-1.0, -1.0, 1.0)),  Vertex(glm::vec3(1.0, -1.0, 1.0)),
      Vertex(glm::vec3(1.0, 1.0, 1.0)),    Vertex(glm::vec3(-1.0, 1.0, 1.0)),
      Vertex(glm::vec3(-1.0, -1.0, -1.0)), Vertex(glm::vec3(1.0, -1.0, -1.0)),
      Vertex(glm::vec3(1.0, 1.0, -1.0)),   Vertex(glm::vec3(-1.0, 1.0, -1.0))};
  std::vector<GLuint> indices = {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1,
                                 7, 6, 5, 5, 4, 7, 4, 0, 3, 3, 7, 4,
                                 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};

  return std::make_shared<TriangleMesh>(vertices, indices, true);
}

void LineMesh::draw() const {
  glBindVertexArray(getVAO());
  glDrawElements(GL_LINES, getIndices().size(), GL_UNSIGNED_INT, 0);
}

DynamicMesh::DynamicMesh(int maxvcount)
    : vertexCount(0), maxVertexCount(maxvcount) {

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glBufferData(GL_ARRAY_BUFFER, maxVertexCount * 6 * sizeof(float), NULL,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

DynamicMesh::~DynamicMesh() {
  if (vbo)
    glDeleteBuffers(1, &vbo);
  if (vao)
    glDeleteVertexArrays(1, &vao);
}

void DynamicMesh::draw() const {
  glBindVertexArray(getVAO());
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void DynamicMesh::setVertexCount(int vcount) {
  vertexCount = std::max(std::min(vcount, maxVertexCount) / 3 * 3, 0);
}

int DynamicMesh::getVertexCount() const { return vertexCount; }

int DynamicMesh::getMaxVertexCount() const { return maxVertexCount; }

} // namespace Optifuser
