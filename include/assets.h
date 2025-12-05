#pragma once
#include <memory>
#include <string>
#include "world.h"
#include "exports.h"
#include <vector>

struct Texture;
struct Shader;

class ION_API AssetSystem {
  // ID, Asset
  std::map<std::string, std::shared_ptr<Texture>> textures;
  std::map<std::string, std::shared_ptr<Shader>> shaders;
	std::filesystem::path project_root;
public:
  void SetProjectRoot(std::filesystem::path path) {
		project_root = path;
  }
  std::filesystem::path GetProjectRoot() const {
		return project_root;
  }

  template <typename T>
	std::shared_ptr<T> LoadAsset(std::filesystem::path path);
  template <typename T> void DestroyAsset(std::shared_ptr<T> asset);
  std::map<std::string, std::shared_ptr<Texture>>& GetTextures() {
    return textures;
	}
  std::map<std::string, std::shared_ptr<Shader>>& GetShaders() {
    return shaders;
  }

  void Inspector();
};
