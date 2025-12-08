#pragma once
#include "exports.h"
#include "world.h"
#include <map>
#include <memory>
#include <string>

struct Texture;
struct Shader;

namespace ion {
namespace res {
namespace internal {
ION_API extern std::map<std::string, std::shared_ptr<Texture>> textures;
ION_API extern std::map<std::string, std::shared_ptr<Shader>> shaders;
ION_API extern std::map<std::string, std::shared_ptr<GPUData>> gpu_datas;
ION_API extern std::map<std::string, std::shared_ptr<World>> worlds;
ION_API extern std::map<std::string, std::shared_ptr<void>> custom_assets;
ION_API extern std::filesystem::path project_root;
} // namespace internal
inline std::map<std::string, std::shared_ptr<Texture>> &GetTextures() {
  return internal::textures;
}
inline std::map<std::string, std::shared_ptr<Shader>> &GetShaders() {
  return internal::shaders;
}
inline std::map<std::string, std::shared_ptr<GPUData>> &GetGPUData() {
  return internal::gpu_datas;
}
inline std::map<std::string, std::shared_ptr<World>> &GetWorlds() {
  return internal::worlds;
}
inline std::map<std::string, std::shared_ptr<void>> &GetCustomAssets() {
  return internal::custom_assets;
}

ION_API bool CheckApplicationStructure();
ION_API void SetProjectRoot(std::filesystem::path path);
ION_API std::filesystem::path GetProjectRoot();

template <typename T>
ION_API std::shared_ptr<T> CreateAsset(std::filesystem::path path);
template <typename T>
ION_API void SaveAsset(std::filesystem::path path, std::shared_ptr<T> asset);
template <typename T>
ION_API std::shared_ptr<T> LoadAsset(std::filesystem::path path,
                                     bool is_hash = true);
template <typename T> ION_API void DestroyAsset(std::shared_ptr<T> asset);

template <>
ION_API std::shared_ptr<World> CreateAsset<World>(std::filesystem::path path);
template <>
ION_API void SaveAsset<World>(std::filesystem::path path,
                              std::shared_ptr<World> asset);
template <>
ION_API std::shared_ptr<World> LoadAsset<World>(std::filesystem::path path,
                                                bool is_hash);
template <>
ION_API std::shared_ptr<Texture> LoadAsset<Texture>(std::filesystem::path path,
                                                    bool is_hash);
template <>
ION_API std::shared_ptr<Shader> LoadAsset<Shader>(std::filesystem::path path,
                                                  bool is_hash);
template <>
ION_API std::shared_ptr<GPUData> LoadAsset<GPUData>(std::filesystem::path path,
                                                    bool is_hash);
template <>
ION_API void SaveAsset<GPUData>(std::filesystem::path path,
                                std::shared_ptr<GPUData> asset);
} // namespace res
} // namespace ion