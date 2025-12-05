#include "assets.h"
#include "context.h"
#include "shader.h"
#include "texture.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include "development/gui.h"
#include <map>
#include <tinyfiledialogs/tinyfiledialogs.h>
#define STB_IMAGE_IMPLEMENTATION
#include "render.h"
#include "development/uuid.h"
#include "stb_image.h"

static std::string Trim(std::string s) {
  auto is_space = [](unsigned char c) { return std::isspace(c); };
  s.erase(s.begin(), std::ranges::find_if_not(s.begin(), s.end(), is_space));
  s.erase(std::ranges::find_if_not(s.rbegin(), s.rend(), is_space).base(),
          s.end());
  return s;
}

// Expects format key:value
static std::map<std::string, std::string> ParseManifest(std::filesystem::path path) {
  std::map<std::string, std::string> values{};
  std::ifstream file{};
  file.open(path);
  if (!file.is_open())
    return values;
  std::string line{};
  while (std::getline(file, line)) {
    line = Trim(line);
    if (line.empty() || line.starts_with('#'))
      continue;

    auto colon_pos = line.find(':');
    if (colon_pos == std::string::npos)
      continue;
    std::string key = Trim(line.substr(0, colon_pos));
    std::string value = Trim(line.substr(colon_pos + 1));
    values[key] = value;
  }
  return values;
}

template <>
std::shared_ptr<Texture>
AssetSystem::LoadAsset<Texture>(std::filesystem::path path) {
	// Check if texture exists
  if (!std::filesystem::exists(path)) {
    printf("Texture manifest does not exist: %s\n", path.string().c_str());
    return nullptr;
  }
	// Create a guid for the texture
	auto guid = ion::uuid::GenerateUUID();
  printf("Copying asset from %s as %s\n", std::filesystem::absolute(path).string().c_str(), guid.c_str());
  std::filesystem::copy_file(std::filesystem::absolute(path), GetProjectRoot() / guid, std::filesystem::copy_options::update_existing);
  TextureInfo info{};
  info.data = stbi_load((GetProjectRoot() / guid).string().c_str(), &info.width, &info.height, &info.nr_channels, 0);
  if (!info.data) {
    printf("Failed to load texture image: %s\n", stbi_failure_reason());
    return nullptr;
  }

  auto texture = std::make_shared<Texture>();
  texture->texture = ion::GetSystem<RenderSystem>().ConfigureTexture(info);
  stbi_image_free(info.data);
  textures.push_back(texture);
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
	// Copy over the shader files to the project assets with a guid
	auto guid = ion::uuid::GenerateUUID();
	
  auto shader_directory = GetProjectRoot() / guid;
	std::filesystem::create_directory(shader_directory);

	std::filesystem::copy_file(abs_path / "vs.glsl",
    shader_directory / "vs.glsl",
    std::filesystem::copy_options::update_existing);

	std::filesystem::copy_file(abs_path / "fs.glsl",
    shader_directory / "fs.glsl",
    std::filesystem::copy_options::update_existing);

  auto shader = std::make_shared<Shader>(shader_directory / "vs.glsl", shader_directory / "fs.glsl");
  shaders.push_back(shader);
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

void AssetSystem::Inspector() {
  ION_GUI_PREP_CONTEXT();
  ImGui::Begin("Asset System");
  if (ImGui::Button("Load Image")) {
    auto file_char = tinyfd_openFileDialog("Load Image", nullptr,
                                           0, nullptr, nullptr, false);
    if (file_char) {
      LoadAsset<Texture>(std::filesystem::path(file_char));
    }
  }
  for (int i = 0; i < textures.size(); i++) {
    ImGui::PushID(i);
    ImGui::Image(textures[i]->texture, ImVec2(100, 100));
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("TEXTURE_ASSET", &textures[i], sizeof(std::shared_ptr<Texture>&));
      ImGui::EndDragDropSource();
    }
    ImGui::PopID();
  }
  ImGui::End();
}
