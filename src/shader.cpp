#include "shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
namespace Optifuser {
Shader::Shader(const GLchar *vertexPath, const GLchar *fragmentPath) {
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertexPath, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << VertexShaderStream.rdbuf();
    VertexShaderCode = sstr.str();
    VertexShaderStream.close();
  } else {
    printf("Impossible to open %s. Are you in the right directory ? Don't "
           "forget to read the FAQ !\n",
           vertexPath);
    getchar();
    return;
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragmentPath, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::stringstream sstr;
    sstr << FragmentShaderStream.rdbuf();
    FragmentShaderCode = sstr.str();
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertexPath);
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    printf("%s\n", &VertexShaderErrorMessage[0]);
  }

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragmentPath);
  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    printf("%s\n", &FragmentShaderErrorMessage[0]);
  }

  // Link the program
  printf("Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    printf("%s\n", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  Id = ProgramID;
}

Shader::~Shader() { glDeleteProgram(Id); }

void Shader::use() const { glUseProgram(Id); }

void Shader::setBool(const std::string &name, bool value) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1)
    glUniform1i(variableId, (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1)
    glUniform1i(variableId, value);
}

void Shader::setFloat(const std::string &name, float value) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1)
    glUniform1f(variableId, value);
}

void Shader::setMatrix(const std::string &name, const glm::mat4 &mat, bool transpose) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1)
    glUniformMatrix4fv(variableId, 1, transpose, &mat[0][0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1)
    glUniform3f(variableId, vec[0], vec[1], vec[2]);
}

void Shader::setUserData(const std::string &name, uint32_t size, float const *data) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (size > 16) {
    std::cerr << "Only 16 floats are allowed in user data, forcing this constraint" << std::endl;
    size = 16;
  }
  glm::mat4 mat(0);
  for (uint32_t i = 0; i < size; ++i) {
    mat[i / 4][i % 4] = data[i];
  }
  if (variableId != -1) {
    glUniformMatrix4fv(variableId, 1, GL_FALSE, &mat[0][0]);
  }
}

void Shader::setTexture(const std::string &name, GLuint textureId, GLint n) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1) {
    glUniform1i(variableId, n);
    glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_2D, textureId);
  }
}

void Shader::setCubemap(const std::string &name, GLuint textureId, GLint n) const {
  GLint variableId = glGetUniformLocation(Id, name.c_str());
  if (variableId != -1) {
    glUniform1i(variableId, n);
    glActiveTexture(GL_TEXTURE0 + n);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
  }
}

} // namespace Optifuser
