#pragma once
#include <string_view>
#include <memory>

struct Texture {
public:
  unsigned int texture = 0;
  void Use();
};

struct TexturePack {
public:
  std::shared_ptr<Texture> color;
  std::shared_ptr<Texture> normal;
  void Use();
};