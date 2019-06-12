#include "noise/perlin.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Optifuser {

template<typename T>
T lerp(T x, T y, T r) {
  return (1-r) * x + r * y;
}

float PerlinNoise::noise(float x, float y, const std::function<unsigned int(long, long)>& hash) {
  long x1 = (long)floor(x); long x2 = x1 + 1;
  long y1 = (long)floor(y); long y2 = y1 + 1;

  float h11 = glm::dot(g(x1, y1, hash), glm::vec2(x-x1, y-y1));
  float h12 = glm::dot(g(x1, y2, hash), glm::vec2(x-x1, y-y2));
  float h21 = glm::dot(g(x2, y1, hash), glm::vec2(x-x2, y-y1));
  float h22 = glm::dot(g(x2, y2, hash), glm::vec2(x-x2, y-y2));
  float h1 = lerp(h11, h12, y-y1);
  float h2 = lerp(h21, h22, y-y1);
  float h = lerp(h1, h2, x-x1);

  return h;
}

PerlinNoise::PerlinNoise(unsigned int gradRes) : gradRes(gradRes) {
  float d = 2 * M_PI / gradRes;
  for (unsigned int i = 0; i < gradRes; i++) {
    grad.push_back(glm::vec2(cos(d*i), sin(d*i)));
  }
}

glm::vec2 PerlinNoise::g(long x, long y, const std::function<unsigned int(long, long)>& hash) {
  return grad[hash(x, y) % gradRes];
}



float PerlinNoise::operator()(float x, float y) {
  float s = 0;
  for (size_t i = 0; i < amplitudes.size(); i++) {
    s += amplitudes[i] * noise(frequencies[i] * (x - translations[i].x),
                               frequencies[i] * (y - translations[i].y),
                               hashes[i]);
  }
  return s;
}

void PerlinNoise::addNoise(float amplitude, glm::vec2 translation, float frequency, std::function<float(int, int)> hash) {
  amplitudes.push_back(amplitude);
  translations.push_back(translation);
  frequencies.push_back(frequency);
  hashes.push_back(hash);
}

void PerlinNoise::addNoise(float amplitude, glm::vec2 translation, float frequency, int seed) {
  amplitudes.push_back(amplitude);
  translations.push_back(translation);
  frequencies.push_back(frequency);
  hashes.push_back([=](long x, long y) {
    std::mt19937 mt((seed << 16) + (x << 8) + y);
    return mt() % this->gradRes;
  });
}

} // namespace Optifuser
