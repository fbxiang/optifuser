#pragma once
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
namespace Optifuser {
class Shader {
public:
  GLuint Id;

  Shader(const GLchar *vertexPath, const GLchar *fragmenetPath);
  ~Shader();

  void use() const;

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setMatrix(const std::string &name, const glm::mat4 &mat,
                 bool transpose = GL_FALSE) const;
  void setVec3(const std::string &name, const glm::vec3 &vec) const;
  void setTexture(const std::string &name, GLuint textureId, GLint n) const;
  void setCubemap(const std::string &name, GLuint textureId, GLint n) const;
};

} // namespace Optifuser
