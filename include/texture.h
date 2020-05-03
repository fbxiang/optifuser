#pragma once
#include <GL/glew.h>
#include <fstream>
#include <memory>
#include <vector>

namespace Optifuser {

std::tuple<std::vector<unsigned char>, int, int, int> load_image(std::string const &filename);
std::tuple<std::vector<float>, int, int, int> load_hdr(std::string const &filename);

class Texture {
private:
  GLuint id = 0;
  int mWidth = 0;
  int mHeight = 0;

public:
  Texture() {}
  virtual ~Texture() { destroy(); }

  void load(const std::string &filename, int mipmap = 0, int wrapping = GL_REPEAT,
            int minFilter = GL_NEAREST_MIPMAP_LINEAR, int magFilter = GL_LINEAR);
  void loadFloat(std::vector<float> const &data, int width, int height, int wrapping = GL_REPEAT,
                 int minFilter = GL_NEAREST_MIPMAP_LINEAR, int magFilter = GL_LINEAR);
  void destroy();
  inline GLuint getId() const { return id; };

  inline int getWidth() const { return mWidth; }
  inline int getHeight() const { return mHeight; }

public:
  static const Texture Empty;
};

std::shared_ptr<Texture> CreateRandomTexture(int width, int height, int seed);

std::shared_ptr<Texture> LoadTexture(const std::string &filename, int mipmap = 0,
                                     int wrapping = GL_REPEAT,
                                     int minFilter = GL_NEAREST_MIPMAP_LINEAR,
                                     int magFilter = GL_LINEAR);

void writeToFile(GLuint textureId, GLuint width, GLuint height, std::string filename);

class CubeMapTexture {
private:
  GLuint id;
  int width = 0;
  int height = 0;

public:
  CubeMapTexture() : id(0) {}
  virtual ~CubeMapTexture() { destroy(); }

  void destroy() {
    glDeleteTextures(1, &id);
    id = 0;
  }

  void load(const std::string &front, const std::string &back, const std::string &top,
            const std::string &bottom, const std::string &left, const std::string &right,
            int wrapping, int filtering);

  inline GLuint getId() const { return id; }
  inline int getWidth() const { return width; }
  inline int getHeight() const { return height; }

private:
  void loadCubeMapSide(GLuint texture, GLenum side, const std::string &filename,
                       bool texStorage = false);
};

std::shared_ptr<CubeMapTexture>
LoadCubeMapTexture(const std::string &front, const std::string &back, const std::string &top,
                   const std::string &bottom, const std::string &left, const std::string &right,
                   int wrapping, int filtering);

void writeTextureRGBAFloat32Raw(GLuint textureId, GLuint width, GLuint height,
                                const std::string &filename);
void writeTextureDepthFloat32Raw(GLuint textureId, GLuint width, GLuint height,
                                 const std::string &filename);

std::vector<float> getDepthFloat32Texture(GLuint textureId, GLuint width, GLuint height);
std::vector<float> getRGBAFloat32Texture(GLuint textureId, GLuint width, GLuint height);
std::vector<int> getInt32Texture(GLuint textureId, GLuint width, GLuint height);

} // namespace Optifuser
