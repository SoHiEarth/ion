#include "ion/development/id.h"

std::string ion::id::GenerateUUID() {
  std::stringstream ss;
  int i;
  ss << std::hex;
  for (i = 0; i < 8; i++) {
    ss << distribution(gen);
  }
  ss << "-";
  for (i = 0; i < 4; i++) {
    ss << distribution(gen);
  }
  ss << "-4";
  for (i = 0; i < 3; i++) {
    ss << distribution(gen);
  }
  ss << "-";
  ss << distribution_2(gen);
  for (i = 0; i < 3; i++) {
    ss << distribution(gen);
  }
  ss << "-";
  for (i = 0; i < 12; i++) {
    ss << distribution(gen);
  };
  return ss.str();
}

std::string ion::id::GenerateHashFromString(const std::string& input) {
  std::hash<std::string> hasher;
  size_t hash = hasher(input);
  std::stringstream ss;
  ss << std::hex << hash;
  return ss.str();
}