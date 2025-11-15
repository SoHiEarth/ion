#include "assets.h"
#include "imgui.h"
#include "shader.h"
#include "texture.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <tinyfiledialogs/tinyfiledialogs.h>

static std::string Trim(std::string s) {
  auto is_space = [](unsigned char c) { return std::isspace(c); };
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), is_space));
  s.erase(std::find_if_not(s.rbegin(), s.rend(), is_space).base(), s.end());
  return s;
}

// Expects format key:value
std::map<std::string, std::string> ParseManifest(std::string_view path) {
  std::map<std::string, std::string> values;
  std::ifstream file;
  file.open(path.data());
  if (!file.is_open())
    return values;
  std::string line;
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

template <> Texture AssetSystem::LoadAsset<Texture>(std::string_view manifest) {
  auto manifest_info = ParseManifest(manifest);
  if (manifest_info["relative"] == "true") {
    auto directory = std::filesystem::path(manifest).parent_path().generic_string();
    auto texture = Texture(directory + manifest_info["path"]);
    textures.push_back(texture);
    return texture;
  } else {
    auto texture = Texture(manifest_info["path"]);
    textures.push_back(texture);
    return texture;
  }
}

template <> Shader AssetSystem::LoadAsset<Shader>(std::string_view manifest) {
  auto manifest_info = ParseManifest(manifest);
  if (manifest_info["relative"] == "true") {
    auto directory = std::filesystem::path(manifest).parent_path().generic_string();
    auto shader = Shader(directory + manifest_info["vertex_path"],
                         directory + manifest_info["fragment_path"]);
    shaders.push_back(shader);
    return shader;
  } else {
    auto shader = Shader(manifest_info["vertex_path"], manifest_info["fragment_path"]);
    shaders.push_back(shader);
    return shader;
  }
}

void AssetSystem::Inspector() {
  ImGui::Begin("Asset System");
  if (ImGui::Button("Load Image from Manifest")) {
    auto file_char = tinyfd_openFileDialog("Load Image From Manifest", nullptr,
                                           0, nullptr, nullptr, false);
    if (file_char) {
      LoadAsset<Texture>(file_char);
    }
  }
  for (int i = 0; i < textures.size(); i++) {
    ImGui::PushID(i);
    ImGui::Image(textures[i].texture, ImVec2(100, 100));
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
      ImGui::SetDragDropPayload("TEXTURE_ASSET", &textures[i], sizeof(Texture));
      ImGui::EndDragDropSource();
    }
    ImGui::PopID();
  }
  ImGui::End();
}
