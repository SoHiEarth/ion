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
	std::map<std::string, std::shared_ptr<GPUData>> gpu_datas;
	std::map<std::string, std::shared_ptr<World>> worlds;
	std::map<std::string, std::shared_ptr<void>> custom_assets;
	std::filesystem::path project_root;
public:
  bool CheckApplicationStructure();

  void SetProjectRoot(std::filesystem::path path) {
		project_root = path;
  }
  std::filesystem::path GetProjectRoot() const {
		return project_root;
  }

  template <typename T>	std::shared_ptr<T> CreateAsset(std::filesystem::path path);
  template <typename T>	void SaveAsset(std::filesystem::path path, std::shared_ptr<T> asset);
  template <typename T>	std::shared_ptr<T> LoadAsset(std::filesystem::path path, bool is_hash = true);
  template <typename T> void DestroyAsset(std::shared_ptr<T> asset);
  std::map<std::string, std::shared_ptr<Texture>>& GetTextures() {
    return textures;
	}
  std::map<std::string, std::shared_ptr<Shader>>& GetShaders() {
    return shaders;
  }
  std::map<std::string, std::shared_ptr<GPUData>>& GetGPUData() {
    return gpu_datas;
  }
  std::map<std::string, std::shared_ptr<World>>& GetWorlds() {
    return worlds;
	}
  std::map<std::string, std::shared_ptr<void>>& GetCustomAssets() {
    return custom_assets;
	}
};

template <>
ION_API std::shared_ptr<World> AssetSystem::CreateAsset<World>(std::filesystem::path path);

template <>
ION_API void AssetSystem::SaveAsset<World>(std::filesystem::path path, std::shared_ptr<World> asset);

template <>
ION_API std::shared_ptr<World> AssetSystem::LoadAsset<World>(std::filesystem::path path, bool is_hash);

template <>
ION_API std::shared_ptr<Texture> AssetSystem::LoadAsset<Texture>(std::filesystem::path path, bool is_hash);

template <>
ION_API std::shared_ptr<Shader> AssetSystem::LoadAsset<Shader>(std::filesystem::path path, bool is_hash);
