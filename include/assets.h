#pragma once
#include <string>
#include <vector>

class Texture;
class Shader;

class AssetSystem {
  std::vector<Texture> textures;
  std::vector<Shader> shaders;

public:
  template <typename T> T LoadAsset(std::string_view manifest);
  template <typename T> void DestroyAsset(T asset);

  void Inspector();
};
