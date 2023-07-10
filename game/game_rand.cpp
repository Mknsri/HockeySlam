#include <random>

static float random(float min, float max)
{
  static std::random_device rd; // only used once to initialise (seed) engine
  std::mt19937 rng(
    rd()); // random-number engine used (Mersenne-Twister in this case)
  std::uniform_real_distribution<float> uni(min, max); // guaranteed unbiased

  return uni(rng);
}