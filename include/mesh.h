#pragma once
#include "texture.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
namespace Optifuser {
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;
  glm::vec3 tangent;
  glm::vec3 bitangent;

  Vertex(glm::vec3 p = glm::vec3(0), glm::vec3 n = glm::vec3(0),
         glm::vec2 t = glm::vec2(0), glm::vec3 tan = glm::vec3(0),
         glm::vec3 bitan = glm::vec3(0))
      : position(p), normal(n), texCoord(t), tangent(tan), bitangent(bitan) {}
};

class AbstractMeshBase {
public:
  virtual void draw() const = 0;
};

class DynamicMesh : public AbstractMeshBase {
  GLuint vao;
  GLuint vbo;
  int vertexCount;
  int maxVertexCount;

public:
  DynamicMesh(int maxvcount);
  DynamicMesh(const DynamicMesh &) = delete;
  DynamicMesh &operator=(const DynamicMesh &) = delete;
  virtual ~DynamicMesh();

  inline GLuint getVAO() const { return vao; }
  inline GLuint getVBO() const { return vbo; }

  void setVertexCount(int vcount);
  int getVertexCount() const;
  int getMaxVertexCount() const;
  virtual void draw() const override;
};

class MeshBase : public AbstractMeshBase {
protected:
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

public:
  MeshBase();
  MeshBase(const std::vector<Vertex> &inVertices,
           const std::vector<GLuint> &inIndices);

  MeshBase(const MeshBase &) = delete;
  MeshBase &operator=(const MeshBase &) = delete;

  virtual ~MeshBase();

  GLuint getVAO() const;
  GLuint getVBO() const;
  GLuint getEBO() const;
  const std::vector<Vertex> &getVertices() const;
  const std::vector<GLuint> &getIndices() const;
};

class TriangleMesh : public MeshBase {
public:
  using MeshBase::MeshBase;
  TriangleMesh(const std::vector<Vertex> &inVertices,
               const std::vector<GLuint> &inIndices, bool recalcNormal = false);

  uint64_t size() const { return indices.size() / 3; }

  virtual void draw() const override;

private:
  void recalculateNormals();
};

class LineMesh : public MeshBase {
public:
  using MeshBase::MeshBase;

  virtual void draw() const override;
};

std::shared_ptr<TriangleMesh> NewCubeMesh();

} // namespace Optifuser
