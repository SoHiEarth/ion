#pragma once
#include <string_view>
class Texture {
public:
  unsigned int texture = 0;
  void Load(std::string_view path);
  Texture() {}
  Texture(std::string_view path) { Load(path); }
};
