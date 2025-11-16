#pragma once
#include <string>
#include <vector>
#include <memory>

struct Texture;
struct Shader;
class Context;

class AssetSystem {
  std::vector<std::shared_ptr<Texture>> textures;
  std::vector<std::shared_ptr<Shader>> shaders;

public:
  template <typename T> std::shared_ptr<T> LoadAsset(std::string_view manifest, Context& context);
  template <typename T> void DestroyAsset(std::shared_ptr<T> asset);

  void Inspector();
};
