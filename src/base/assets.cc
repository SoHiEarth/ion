#include "ion/assets.h"
#include "ion/context.h"
#include "ion/shader.h"
#include "ion/texture.h"
#include <pugixml.hpp>
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
#include "ion/save_keys.h"

static bool VerifyPathExists(const std::filesystem::path& path) {
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
          if (entry.is_directory()) {
            std::filesystem::create_directory("assets" / entry.path().filename());
					}
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
AssetSystem::LoadAsset<Texture>(std::filesystem::path path, bool is_hash) {
  if (!is_hash && !std::filesystem::exists(path)) {
    printf("Texture manifest does not exist: %s\n", path.string().c_str());
    return nullptr;
  }
  if (!is_hash) {
    auto id = ion::id::GenerateHashFromString(path.string());
    printf("Copying asset from %s as %s\n", std::filesystem::absolute(path).string().c_str(), id.c_str());
    std::filesystem::copy_file(std::filesystem::absolute(path), GetProjectRoot() / id, std::filesystem::copy_options::update_existing);
    TextureInfo info{};
    info.data = stbi_load((GetProjectRoot() / id).string().c_str(), &info.width, &info.height, &info.nr_channels, 0);
    if (!info.data) {
      printf("Failed to load texture image: %s\n", stbi_failure_reason());
      return nullptr;
    }
    auto texture = std::make_shared<Texture>(id);
    texture->texture = ion::GetSystem<RenderSystem>().ConfigureTexture(info);
    stbi_image_free(info.data);
    textures.insert({ id, texture });
    return texture;
  }
  else {
		path = GetProjectRoot() / path;
    TextureInfo info{};
    info.data = stbi_load(path.string().c_str(), &info.width, &info.height, &info.nr_channels, 0);
    if (!info.data) {
      printf("Failed to load texture image: %s\n", stbi_failure_reason());
      return nullptr;
    }
    auto texture = std::make_shared<Texture>(path.filename().string());
    texture->texture = ion::GetSystem<RenderSystem>().ConfigureTexture(info);
    stbi_image_free(info.data);
    textures.insert({ path.string(), texture });
		return texture;
  }
}

// Path is a directory containing vs.glsl and fs.glsl
template <>
std::shared_ptr<Shader>
AssetSystem::LoadAsset<Shader>(std::filesystem::path path, bool is_hash) {
	if (!is_hash && !std::filesystem::exists(path)) {
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
	if (!is_hash) {
    auto id = ion::id::GenerateHashFromString(path.string());
    auto shader_directory = GetProjectRoot() / id;
    std::filesystem::create_directory(shader_directory);

    std::filesystem::copy_file(abs_path / "vs.glsl",
      shader_directory / "vs.glsl",
      std::filesystem::copy_options::update_existing);

    std::filesystem::copy_file(abs_path / "fs.glsl",
      shader_directory / "fs.glsl",
      std::filesystem::copy_options::update_existing);
    auto shader = std::make_shared<Shader>(shader_directory, id);
    shaders.insert({ id, shader });
		return shader;
  }

  if (is_hash) {
    path = GetProjectRoot() / path;
    auto shader = std::make_shared<Shader>(path, path.filename().string());
		shaders.insert({ path.string(), shader });
		return shader;
  }
}

static void ProcessWorldManifest(std::shared_ptr<World> world) {
	auto path = world->GetWorldPath();
  if (!std::filesystem::exists(path)) {
		throw std::runtime_error("World manifest does not exist: " + path.string());
	}
	auto doc = pugi::xml_document{};
  doc.load_file(path.c_str());
  // version at node declaration
  auto meta = doc.child(ION_SAVE_METADATA);
  if (strcmp(meta.attribute(ION_SAVE_VERSION).as_string(), ION_BUILD_VERSION) != 0) {
		printf("World version mismatch: expected %s, got %s\n",
      ION_BUILD_VERSION,
      meta.attribute(ION_SAVE_VERSION).as_string());
  }
	auto root = doc.child(ION_SAVE_WORLD);
  for (auto marker_node : root.children(ION_SAVE_MARKER_KEY)) {
    auto id = marker_node.attribute(ION_SAVE_MARKER_ID).as_uint();
    auto value = marker_node.attribute(ION_SAVE_MARKER_VAL).as_string();
    world->GetMarkers().insert({id, value});
	}
	for (auto component_node : root.children(ION_SAVE_COMPONENT_KEY)) {
    auto type = std::string(component_node.attribute(ION_SAVE_COMPONENT_TYPE).as_string());
    auto id = component_node.attribute(ION_SAVE_ENTITY_ID).as_uint();
    if (type == ION_SAVE_TRANSFORM_KEY) {
      auto transform = world->NewComponent<Transform>(id);
      auto transform_node = component_node.child(ION_SAVE_TRANSFORM_KEY);
      transform->position.x = transform_node.attribute(ION_SAVE_TRANSFORM_POS_X).as_float();
      transform->position.y = transform_node.attribute(ION_SAVE_TRANSFORM_POS_Y).as_float();
			transform->rotation = transform_node.attribute(ION_SAVE_TRANSFORM_ROTATION).as_float(transform->rotation);
      transform->scale.x = transform_node.attribute(ION_SAVE_TRANSFORM_SCALE_X).as_float(transform->scale.x);
      transform->scale.y = transform_node.attribute(ION_SAVE_TRANSFORM_SCALE_Y).as_float(transform->scale.y);
    }
    if (type == ION_SAVE_RENDERABLE_KEY) {
      auto renderable = world->NewComponent<Renderable>(id);
      auto renderable_node = component_node.child(ION_SAVE_RENDERABLE_KEY);
			renderable->color = ion::GetSystem<AssetSystem>().LoadAsset<Texture>(renderable_node.attribute(ION_SAVE_RENDERABLE_COLOR).as_string());
			renderable->normal = ion::GetSystem<AssetSystem>().LoadAsset<Texture>(renderable_node.attribute(ION_SAVE_RENDERABLE_NORMAL).as_string());
			renderable->shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>(renderable_node.attribute(ION_SAVE_RENDERABLE_SHADER).as_string());
    }
    if (type == ION_SAVE_LIGHT_KEY) {
			auto light = world->NewComponent<Light>(id);
      auto light_node = component_node.child(ION_SAVE_LIGHT_KEY);
			light->type = static_cast<LightType>(light_node.attribute(ION_SAVE_LIGHT_TYPE).as_int());
			light->intensity = light_node.attribute(ION_SAVE_LIGHT_INTENSITY).as_float(light->intensity);
			light->radial_falloff = light_node.attribute(ION_SAVE_LIGHT_RADIAL_FALLOFF).as_float(light->radial_falloff);
			light->volumetric_intensity = light_node.attribute(ION_SAVE_LIGHT_VOLUMETRIC_INTENSITY).as_float(light->volumetric_intensity);
			light->color.r = light_node.attribute(ION_SAVE_LIGHT_COLOR_R).as_float(light->color.r);
			light->color.g = light_node.attribute(ION_SAVE_LIGHT_COLOR_G).as_float(light->color.g);
			light->color.b = light_node.attribute(ION_SAVE_LIGHT_COLOR_B).as_float(light->color.b);
    }
  }
}

template <>
std::shared_ptr<World> AssetSystem::LoadAsset<World>(std::filesystem::path path, bool is_hash) {
	auto world = std::make_shared<World>(path);
	SetProjectRoot(path.parent_path() / "assets");
	if (!std::filesystem::exists(path.parent_path() / "assets")) {
		std::filesystem::create_directory(path.parent_path() / "assets");
  }
	ProcessWorldManifest(world);
  return world;
}