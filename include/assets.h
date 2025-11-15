#pragma once
#include <stduuid/uuid.h>
#include <string>
#include <vector>

class Texture;
class Shader;
enum class AssetType { Texture, Shader };

class Asset {
  AssetType type;
  uuids::uuid uuid;
  const std::string manifest_path;
};

class AssetSystem {
  std::vector<Texture> textures;
  std::vector<Shader> shaders;

public:
  template <typename T> T LoadAsset(std::string_view manifest);
  void DestroyAsset(Asset asset);

  void Inspector();
};
