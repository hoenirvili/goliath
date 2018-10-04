#include "cfgtrace/random/random.h"
#include <chrono>
#include <random>
#include <string>

namespace random
{
std::string string()
{
    auto time_point = std::chrono::high_resolution_clock::now();
    auto since = time_point.time_since_epoch();
    // remove the other half, I don't care
    unsigned int seed = (unsigned int)(since.count() & 0xFFFFFFFF);
    std::mt19937 mt_rand(seed);
    auto random = mt_rand();
    return std::to_string(random);
}

}; // namespace random
