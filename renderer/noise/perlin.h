#include <functional>
#include <glm/glm.hpp>
#include <random>
#include <vector>
namespace Optifuser {
class PerlinNoise {
private:
  unsigned int gradRes;
  std::vector<glm::vec2> grad;

  std::vector<float> amplitudes;
  std::vector<glm::vec2> translations;
  std::vector<float> frequencies;
  std::vector<std::function<int(long, long)>> hashes;

  float noise(float x, float y,
              const std::function<unsigned int(long, long)> &hash);
  glm::vec2 g(long x, long y,
              const std::function<unsigned int(long, long)> &hash);

public:
  PerlinNoise(unsigned int gradRes = 256);

  void addNoise(float amplitude, glm::vec2 translation, float frequency,
                std::function<float(int, int)> hash);
  void addNoise(float amplitude, glm::vec2 translation, float frequency,
                int seed);

  float operator()(float x, float y);
};

} // namespace optifuser
