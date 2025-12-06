#include "ion/assets.h"
#include "ion/context.h"
#include "ion/shader.h"
#include "ion/texture.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include "ion/development/gui.h"
#include <map>
#include <tinyfiledialogs/tinyfiledialogs.h>
#define STB_IMAGE_IMPLEMENTATION
#include "ion/render.h"
#include "ion/development/id.h"
#include "stb_image.h"

bool VerifyPathExists(const std::filesystem::path& path) {
  return std::filesystem::exists(path);
}

bool AssetSystem::CheckApplicationStructure() {
  if (!VerifyPathExists("assets")) {
    printf("Assets directory does not exist, locate it? (y/n)");
		auto response = getchar();
    if (response == 'y') {
      auto dir = tinyfd_selectFolderDialog("Select Assets Directory", nullptr);
      if (dir) {
        std::filesystem::create_directory("assets");
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
          std::filesystem::copy(entry.path(), "assets" / entry.path().filename(), std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        }
      } else {
        return false;
      }
    }
    else {
      return false;
		}
  }
  return true;
}

template <>
std::shared_ptr<Texture>
AssetSystem::LoadAsset<Texture>(std::filesystem::path path) {
	// Check if texture exists
  if (!std::filesystem::exists(path)) {
    printf("Texture manifest does not exist: %s\n", path.string().c_str());
    return nullptr;
  }
	// Create a id for the texture
	auto id = ion::id::GenerateHashFromString(path.string());
  printf("Copying asset from %s as %s\n", std::filesystem::absolute(path).string().c_str(), id.c_str());
  std::filesystem::copy_file(std::filesystem::absolute(path), GetProjectRoot() / id, std::filesystem::copy_options::update_existing);
  TextureInfo info{};
  info.data = stbi_load((GetProjectRoot() / id).string().c_str(), &info.width, &info.height, &info.nr_channels, 0);
  if (!info.data) {
    printf("Failed to load texture image: %s\n", stbi_failure_reason());
    return nullptr;
  }

  auto texture = std::make_shared<Texture>();
  texture->texture = ion::GetSystem<RenderSystem>().ConfigureTexture(info);
  stbi_image_free(info.data);
  textures.insert({ id, texture });
  return texture;
}

// Path is a directory containing vs.glsl and fs.glsl
template <>
std::shared_ptr<Shader>
AssetSystem::LoadAsset<Shader>(std::filesystem::path path) {
	if (!std::filesystem::exists(path)) {
    printf("Shader directory does not exist: %s\n", path.string().c_str());
    return nullptr;
  }
  // Check for directory structure
  auto abs_path = std::filesystem::absolute(path);
  if (!std::filesystem::exists(abs_path / "vs.glsl") ||
      !std::filesystem::exists(abs_path / "fs.glsl")) {
    printf("Shader directory missing vs.glsl or fs.glsl: %s\n", path.string().c_str());
    return nullptr;
	}
	// Copy over the shader files to the project assets with a id
	auto id = ion::id::GenerateHashFromString(path.string());
	
  auto shader_directory = GetProjectRoot() / id;
	std::filesystem::create_directory(shader_directory);

	std::filesystem::copy_file(abs_path / "vs.glsl",
    shader_directory / "vs.glsl",
    std::filesystem::copy_options::update_existing);

	std::filesystem::copy_file(abs_path / "fs.glsl",
    shader_directory / "fs.glsl",
    std::filesystem::copy_options::update_existing);

  auto shader = std::make_shared<Shader>(shader_directory / "vs.glsl", shader_directory / "fs.glsl");
  shaders.insert({id, shader});
  return shader;
}

template <>
std::shared_ptr<World> AssetSystem::LoadAsset<World>(std::filesystem::path path) {
	auto world = std::make_shared<World>(path);
	SetProjectRoot(path.parent_path() / "assets");
	if (!std::filesystem::exists(path.parent_path() / "assets")) {
		std::filesystem::create_directory(path.parent_path() / "assets");
  }
  return world;
}