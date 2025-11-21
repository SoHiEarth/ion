#include "assets.h"
#include "context.h"
#include "shader.h"
#include "texture.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <map>
#include <tinyfiledialogs/tinyfiledialogs.h>
#define STB_IMAGE_IMPLEMENTATION
#include "render.h"
#include "stb_image.h"

static std::string Trim(std::string s) {
  auto is_space = [](unsigned char c) { return std::isspace(c); };
  s.erase(s.begin(), std::ranges::find_if_not(s.begin(), s.end(), is_space));
  s.erase(std::ranges::find_if_not(s.rbegin(), s.rend(), is_space).base(),
          s.end());
  return s;
}

// Expects format key:value
static std::map<std::string, std::string> ParseManifest(std::string_view path) {
  std::map<std::string, std::string> values{};
  std::ifstream file{};
  file.open(path.data());
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
AssetSystem::LoadAsset<Texture>(std::string_view manifest, Context &context) {
  printf("Loading texture from manifest: %s\n", manifest.data());
  TextureInfo info{};
  auto manifest_info = ParseManifest(manifest);
  if (manifest_info["flip"] == "true") {
    stbi_set_flip_vertically_on_load(true);
  }
  std::string image_path;
  if (manifest_info["relative"] == "true") {
    auto directory =
        std::filesystem::path(manifest).parent_path().generic_string();
    image_path = directory + manifest_info["path"];
  } else {
    image_path = manifest_info["path"];
  }
  printf("Loading texture image: %s\n", image_path.c_str());
  info.data = stbi_load(image_path.c_str(), &info.width, &info.height, 
                        &info.nr_channels, manifest_info["force_channels"] == "true" ?
                        std::stoi(manifest_info["channels"]) : 0);
  if (!info.data) {
    const char* reason = stbi_failure_reason();
    printf("Failed to load texture image: %s\n", reason);
  }

  auto texture = std::make_shared<Texture>();
  texture->texture = context.render_sys.ConfigureTexture(info);
  stbi_image_free(info.data);
  textures.push_back(texture);
  return texture;
}

template <>
std::shared_ptr<TexturePack>
AssetSystem::LoadAsset<TexturePack>(std::string_view manifest, Context &context) {
  TexturePack pack{};
  auto manifest_info = ParseManifest(manifest);
  std::string directory;
  if (manifest_info["relative"] == "true") {
    directory = std::filesystem::path(manifest).parent_path().generic_string();
  }
  pack.color = LoadAsset<Texture>(directory + manifest_info["color"], context);
  pack.normal = LoadAsset<Texture>(directory + manifest_info["normal"], context);
  return std::make_shared<TexturePack>(pack);
}

template <>
std::shared_ptr<Shader>
AssetSystem::LoadAsset<Shader>(std::string_view manifest, Context &context) {
  auto manifest_info = ParseManifest(manifest);
  if (manifest_info["relative"] == "true") {
    auto directory =
        std::filesystem::path(manifest).parent_path().generic_string();
    auto shader =
        std::make_shared<Shader>(directory + manifest_info["vertex_path"],
                                 directory + manifest_info["fragment_path"]);
    shaders.push_back(shader);
    return shader;
  } else {
    auto shader = std::make_shared<Shader>(manifest_info["vertex_path"],
                                           manifest_info["fragment_path"]);
    shaders.push_back(shader);
    return shader;
  }
}

template <>
std::shared_ptr<World> AssetSystem::LoadAsset<World>(std::string_view manifest, Context& context) {
	auto world = std::make_shared<World>();
  return world;
}
void AssetSystem::Inspector() {
  ImGui::Begin("Asset System");
  if (ImGui::Button("Load Image from Manifest")) {
    auto file_char = tinyfd_openFileDialog("Load Image From Manifest", nullptr,
                                           0, nullptr, nullptr, false);
    if (file_char) {
      LoadAsset<Texture>(file_char, Context::Get());
    }
  }
  if (ImGui::Button("Load Texture Pack from Manifest")) {
    auto file_char = tinyfd_openFileDialog(
        "Load Texture Pack From Manifest", nullptr, 0, nullptr, nullptr, false);
    if (file_char) {
      LoadAsset<TexturePack>(file_char, Context::Get());
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
