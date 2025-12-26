#pragma once
#include <random>
#include <sstream>

namespace ion {
  namespace id {
    static std::random_device              device;
    static std::mt19937                    gen(device());
    static std::uniform_int_distribution<> distribution(0, 15);
    static std::uniform_int_distribution<> distribution_2(8, 11);

    std::string GenerateUUID();
    std::string GenerateHashFromString(const std::string& input);
	} // namespace uuid
}  // namespace ion