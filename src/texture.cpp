#include "texture.h"
#include "debug.h"
#include <iostream>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
namespace Optifuser {
const Texture Texture::Empty;

void Texture::load(const std::string &filename, int mipmap, int wrapping, int minFilter,
                   int magFilter) {
  if (id)
    destroy();

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

  glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

  stbi_image_free(data);
  LABEL_TEXTURE(id, filename);

  mWidth = width;
  mHeight = height;
}

void Texture::loadFloat(std::vector<float> const &data, int width, int height, int wrapping,
                        int minFilter, int magFilter) {
  if (id)
    destroy();

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, data.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

  mWidth = width;
  mHeight = height;
}

void Texture::destroy() {
  glDeleteTextures(1, &id);
  id = 0;
  mWidth = 0;
  mHeight = 0;
}

std::shared_ptr<Texture> CreateRandomTexture(int width, int height, int seed) {
  auto tex = std::make_shared<Texture>();
  glActiveTexture(GL_TEXTURE0);

  std::mt19937 mt(seed);
  std::uniform_real_distribution<float> dist(0, 1);

  std::vector<float> r(width * height);
  for (int i = 0; i < width * height; ++i) {
    r[i] = dist(mt);
  }

  tex->loadFloat(r, width, height);

  return tex;
}

std::shared_ptr<Texture> LoadTexture(const std::string &filename, int mipmap, int wrapping,
                                     int minFilter, int maxFilter) {
  auto tex = std::make_shared<Texture>();
  tex->load(filename, mipmap, wrapping, minFilter, maxFilter);
  return tex;
}

void writeToFile(GLuint textureId, GLuint width, GLuint height, std::string filename) {
  uint8_t data[width * height * 4];
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < 4 * width; ++i) {
      std::swap(data[h1 * width * 4 + i], data[h2 * width * 4 + i]);
    }
  }

  stbi_write_png(filename.c_str(), width, height, 4, data, width * 4);
}

void CubeMapTexture::loadCubeMapSide(GLuint texture, GLenum side, const std::string &filename,
                                     bool texStorage) {
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 4);

  if (this->width != 0 && (width != this->width || height != this->height)) {
    std::cerr << "Cubemap is broken" << std::endl;
    return;
  }
  this->width = width;
  this->height = height;

  if (texStorage) {
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, width, height);
  }
  glTexSubImage2D(side, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

  stbi_image_free(data);
}

void CubeMapTexture::load(const std::string &front, const std::string &back,
                          const std::string &top, const std::string &bottom,
                          const std::string &left, const std::string &right, int wrapping,
                          int filtering) {
  if (id)
    destroy();

  glGenTextures(1, &id);
  printf("Cube map generated: %d\n", id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right, true);
  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
  loadCubeMapSide(id, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filtering);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filtering);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapping);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapping);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapping);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

std::shared_ptr<CubeMapTexture>
LoadCubeMapTexture(const std::string &front, const std::string &back, const std::string &top,
                   const std::string &bottom, const std::string &left, const std::string &right,
                   int wrapping, int filtering) {
  auto tex = std::make_shared<CubeMapTexture>();
  tex->load(front, back, top, bottom, left, right, wrapping, filtering);
  printf("Cube map loaded\n");
  return tex;
}

void writeTextureRGBAFloat32Raw(GLuint textureId, GLuint width, GLuint height,
                                const std::string &filename) {
  float *data = new float[width * height * 4];
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < 4 * width; ++i) {
      std::swap(data[h1 * width * 4 + i], data[h2 * width * 4 + i]);
    }
  }
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<char *>(data), width * height * 4 * sizeof(float));
  file.close();
  delete[] data;
  std::ofstream metafile(filename + ".meta");
  metafile << "width: " << width << ", height: " << height << "\n";
  metafile.close();
}

void writeTextureDepthFloat32Raw(GLuint textureId, GLuint width, GLuint height,
                                 const std::string &filename) {
  float *data = new float[width * height];
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < width; ++i) {
      std::swap(data[h1 * width + i], data[h2 * width + i]);
    }
  }
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  file.write(reinterpret_cast<char *>(data), width * height * sizeof(float));
  file.close();
  delete[] data;
  std::ofstream metafile(filename + ".meta");
  metafile << "width: " << width << ", height: " << height << "\n";
  metafile.close();
}

std::vector<float> getDepthFloat32Texture(GLuint textureId, GLuint width, GLuint height) {
  std::vector<float> output(width * height);
  float *data = output.data();
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < width; ++i) {
      std::swap(data[h1 * width + i], data[h2 * width + i]);
    }
  }
  return output;
}

std::vector<float> getRGBAFloat32Texture(GLuint textureId, GLuint width, GLuint height) {
  std::vector<float> output(width * height * 4);
  float *data = output.data();
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < 4 * width; ++i) {
      std::swap(data[h1 * width * 4 + i], data[h2 * width * 4 + i]);
    }
  }
  return output;
}

std::vector<int> getInt32Texture(GLuint textureId, GLuint width, GLuint height) {
  std::vector<int> output(width * height);
  int *data = output.data();
  glBindTexture(GL_TEXTURE_2D, textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_INT, data);
  for (uint32_t h1 = 0; h1 < height / 2; ++h1) {
    uint32_t h2 = height - 1 - h1;
    for (uint32_t i = 0; i < width; ++i) {
      std::swap(data[h1 * width + i], data[h2 * width + i]);
    }
  }
  return output;
}

std::tuple<std::vector<unsigned char>, int, int, int> load_image(std::string const &filename) {
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
  auto vec = std::vector<unsigned char>(data, data + width * height * 4);
  stbi_image_free(data);
  return {vec, width, height, 4};
}

std::tuple<std::vector<float>, int, int, int> load_hdr(std::string const &filename) {
  int width, height, nrChannels;
  float *data = stbi_loadf(filename.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
  auto vec = std::vector<float>(data, data + width * height * 4);
  stbi_image_free(data);
  return {vec, width, height, 4};
}

} // namespace Optifuser
