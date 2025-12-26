#include "ion/assets.h"
#include "ion/development/gui.h"
#include "ion/shader.h"
#include "ion/texture.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <imgui.h>
#include <map>
#include <pugixml.hpp>
#include <tinyfiledialogs/tinyfiledialogs.h>
#define STB_IMAGE_IMPLEMENTATION
#include "ion/development/id.h"
#include "ion/gpu_data.h"
#include "ion/physics.h"
#include "ion/render.h"
#include "ion/save_keys.h"
#include "stb_image.h"

namespace ion::res::internal {
ION_API std::map<std::string, std::shared_ptr<Texture>> textures;
ION_API std::map<std::string, std::shared_ptr<Shader>> shaders;
ION_API std::map<std::string, std::shared_ptr<GPUData>> gpu_datas;
ION_API std::map<std::string, std::shared_ptr<World>> worlds;
ION_API std::map<std::string, std::shared_ptr<void>> custom_assets;
ION_API std::filesystem::path project_root;
} // namespace ion::res::internal

static void ProcessWorldManifest(std::shared_ptr<World> world) {
  auto path = world->GetWorldPath();
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("World manifest does not exist: " + path.string());
  }
  auto doc = pugi::xml_document{};
  doc.load_file(path.c_str());
  auto meta = doc.child(ION_SAVE_METADATA);
  if (strcmp(meta.attribute(ION_SAVE_VERSION).as_string(), ION_BUILD_VERSION) !=
      0) {
    printf("World version mismatch: expected %s, got %s\n", ION_BUILD_VERSION,
           meta.attribute(ION_SAVE_VERSION).as_string());
  }
  auto root = doc.child(ION_SAVE_WORLD);
  for (auto marker_node : root.children(ION_SAVE_MARKER_KEY)) {
    auto id = marker_node.attribute(ION_SAVE_MARKER_ID).as_uint();
    auto value = marker_node.attribute(ION_SAVE_MARKER_VAL).as_string();
    world->GetMarkers().insert({id, value});
  }
  for (auto component_node : root.children(ION_SAVE_COMPONENT_KEY)) {
    auto type = std::string(
        component_node.attribute(ION_SAVE_COMPONENT_TYPE).as_string());
    auto id = component_node.attribute(ION_SAVE_ENTITY_ID).as_uint();
    if (type == ION_SAVE_TRANSFORM_KEY) {
      auto transform = world->NewComponent<Transform>(id);
      auto transform_node = component_node.child(ION_SAVE_TRANSFORM_KEY);
      transform->position.x =
          transform_node.attribute(ION_SAVE_TRANSFORM_POS_X).as_float();
      transform->position.y =
          transform_node.attribute(ION_SAVE_TRANSFORM_POS_Y).as_float();
      transform->rotation =
          transform_node.attribute(ION_SAVE_TRANSFORM_ROTATION)
              .as_float(transform->rotation);
      transform->scale.x = transform_node.attribute(ION_SAVE_TRANSFORM_SCALE_X)
                               .as_float(transform->scale.x);
      transform->scale.y = transform_node.attribute(ION_SAVE_TRANSFORM_SCALE_Y)
                               .as_float(transform->scale.y);
    } else if (type == ION_SAVE_RENDERABLE_KEY) {
      auto renderable = world->NewComponent<Renderable>(id);
      auto renderable_node = component_node.child(ION_SAVE_RENDERABLE_KEY);
      renderable->color = ion::res::LoadAsset<Texture>(
          renderable_node.attribute(ION_SAVE_RENDERABLE_COLOR).as_string());
      renderable->normal = ion::res::LoadAsset<Texture>(
          renderable_node.attribute(ION_SAVE_RENDERABLE_NORMAL).as_string());
      renderable->shader = ion::res::LoadAsset<Shader>(
          renderable_node.attribute(ION_SAVE_RENDERABLE_SHADER).as_string());
      renderable->data = ion::res::LoadAsset<GPUData>(
          renderable_node.attribute(ION_SAVE_RENDERABLE_GPU_DATA).as_string());
    } else if (type == ION_SAVE_PHYSICS_BODY_KEY) {
      auto physics_body = world->NewComponent<PhysicsBody>(id);
      auto physics_node = component_node.child(ION_SAVE_PHYSICS_BODY_KEY);
      physics_body->body_id =
          ion::physics::CreateBody(world->GetComponent<Transform>(id));
    } else if (type == ION_SAVE_LIGHT_KEY) {
      auto light = world->NewComponent<Light>(id);
      auto light_node = component_node.child(ION_SAVE_LIGHT_KEY);
      light->type = static_cast<LightType>(
          light_node.attribute(ION_SAVE_LIGHT_TYPE).as_int());
      light->intensity = light_node.attribute(ION_SAVE_LIGHT_INTENSITY)
                             .as_float(light->intensity);
      light->radial_falloff =
          light_node.attribute(ION_SAVE_LIGHT_RADIAL_FALLOFF)
              .as_float(light->radial_falloff);
      light->volumetric_intensity =
          light_node.attribute(ION_SAVE_LIGHT_VOLUMETRIC_INTENSITY)
              .as_float(light->volumetric_intensity);
      light->color.r =
          light_node.attribute(ION_SAVE_LIGHT_COLOR_R).as_float(light->color.r);
      light->color.g =
          light_node.attribute(ION_SAVE_LIGHT_COLOR_G).as_float(light->color.g);
      light->color.b =
          light_node.attribute(ION_SAVE_LIGHT_COLOR_B).as_float(light->color.b);
    } else if (type == ION_SAVE_CAMERA_KEY) {
      auto camera = world->NewComponent<Camera>(id);
    }
  }
}
static DataType StringToDataType(std::string_view str) {
  static const std::map<std::string, DataType> type_map = {
      {"INT", DataType::INT},
      {"UINT", DataType::UNSIGNED_INT},
      {"FLOAT", DataType::FLOAT}};
  auto it = type_map.find(str.data());
  return it != type_map.end() ? it->second : DataType::FLOAT;
}
static DataDescriptor LoadGPUDataManifest(std::filesystem::path path) {
  if (!std::filesystem::exists(path)) {
    printf("GPUData manifest does not exist: %s\n", path.string().c_str());
    return {};
  }
  auto doc = pugi::xml_document{};
  doc.load_file(path.c_str());
  auto root = doc.child("GPUData");
  DataDescriptor descriptor{};
  for (auto pointer_node : root.children("AttributePointer")) {
    AttributePointer pointer{};
    pointer.size = pointer_node.attribute("size").as_int();
    pointer.type = StringToDataType(pointer_node.attribute("type").as_string());
    pointer.normalized = pointer_node.attribute("normalized").as_bool();
    pointer.stride =
        static_cast<size_t>(pointer_node.attribute("stride").as_ullong());
    pointer.pointer = reinterpret_cast<const void *>(
        pointer_node.attribute("pointer").as_ullong());
    descriptor.pointers.push_back(pointer);
  }
  descriptor.element_enabled = root.attribute("element_enabled").as_bool();
  for (auto vertex_node : root.child("vertices").children("vertex")) {
    descriptor.vertices.push_back(vertex_node.attribute("val").as_float());
  }
  for (auto index_node : root.child("indices").children("index")) {
    descriptor.indices.push_back(index_node.attribute("val").as_uint());
  }
  return descriptor;
}

ION_API bool ion::res::CheckApplicationStructure() {
  if (!std::filesystem::exists("assets")) {
    printf("Assets directory does not exist, locate it? (y/n): ");
    auto response = getchar();
    if (response == 'y') {
      auto dir = tinyfd_selectFolderDialog("Select Assets Directory", nullptr);
      if (std::filesystem::exists({dir})) {
        std::filesystem::create_directory("assets");
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
          if (entry.is_directory()) {
            std::filesystem::create_directory("assets" /
                                              entry.path().filename());
          }
          std::filesystem::copy(
              entry.path(), "assets" / entry.path().filename(),
              std::filesystem::copy_options::recursive |
                  std::filesystem::copy_options::update_existing);
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}
ION_API void ion::res::SetProjectRoot(std::filesystem::path path) {
  internal::project_root = path;
}
ION_API std::filesystem::path ion::res::GetProjectRoot() {
  return internal::project_root;
}

template <>
ION_API std::shared_ptr<World>
ion::res::CreateAsset<World>(std::filesystem::path path) {
  if (std::filesystem::exists(path)) {
    printf("World already exists at path: %s\n", path.string().c_str());
    return nullptr;
  }
  auto world = std::make_shared<World>(path);
  SetProjectRoot(path.parent_path() / "assets");
  if (!std::filesystem::exists(path.parent_path() / "assets")) {
    std::filesystem::create_directory(path.parent_path() / "assets");
  }

  SaveAsset<World>(path, world);
  internal::worlds.insert(
      {ion::id::GenerateHashFromString(path.string()), world});
  return world;
}
template <>
ION_API void ion::res::SaveAsset(std::filesystem::path path,
                                 std::shared_ptr<World> asset) {
  auto doc = pugi::xml_document();
  auto declaration = doc.append_child(ION_SAVE_METADATA);
  declaration.append_attribute(ION_SAVE_VERSION) = ION_BUILD_VERSION;
  auto root = doc.append_child(ION_SAVE_WORLD);
  for (const auto &[entity_id, marker_name] : asset->GetMarkers()) {
    auto marker_node = root.append_child(ION_SAVE_MARKER_KEY);
    marker_node.append_attribute(ION_SAVE_MARKER_ID) = entity_id;
    marker_node.append_attribute(ION_SAVE_MARKER_VAL) = marker_name.c_str();
  }
  for (const auto &[entity_id, transform] :
       asset->GetComponentSet<Transform>()) {
    auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
    component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) =
        ION_SAVE_TRANSFORM_KEY;
    component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
    auto transform_node = component_node.append_child(ION_SAVE_TRANSFORM_KEY);
    transform_node.append_attribute(ION_SAVE_TRANSFORM_POS_X) =
        transform->position.x;
    transform_node.append_attribute(ION_SAVE_TRANSFORM_POS_Y) =
        transform->position.y;
    transform_node.append_attribute(ION_SAVE_TRANSFORM_ROTATION) =
        transform->rotation;
    transform_node.append_attribute(ION_SAVE_TRANSFORM_SCALE_X) =
        transform->scale.x;
    transform_node.append_attribute(ION_SAVE_TRANSFORM_SCALE_Y) =
        transform->scale.y;
  }
  for (const auto &[entity_id, renderable] :
       asset->GetComponentSet<Renderable>()) {
    auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
    component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) =
        ION_SAVE_RENDERABLE_KEY;
    component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
    auto renderable_node = component_node.append_child(ION_SAVE_RENDERABLE_KEY);
    renderable_node.append_attribute(ION_SAVE_RENDERABLE_COLOR) =
        renderable->color->GetID().c_str();
    renderable_node.append_attribute(ION_SAVE_RENDERABLE_NORMAL) =
        renderable->normal->GetID().c_str();
    renderable_node.append_attribute(ION_SAVE_RENDERABLE_SHADER) =
        renderable->shader->GetID().c_str();
    renderable_node.append_attribute(ION_SAVE_RENDERABLE_GPU_DATA) =
        renderable->data->GetID().c_str();
  }
  for (const auto &[entity_id, physics_body] :
       asset->GetComponentSet<PhysicsBody>()) {
    auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
    component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) =
        ION_SAVE_PHYSICS_BODY_KEY;
    component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
    auto physics_node = component_node.append_child(ION_SAVE_PHYSICS_BODY_KEY);
  }
  for (const auto &[entity_id, light] : asset->GetComponentSet<Light>()) {
    auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
    component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) =
        ION_SAVE_LIGHT_KEY;
    component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
    auto light_node = component_node.append_child(ION_SAVE_LIGHT_KEY);
    light_node.append_attribute(ION_SAVE_LIGHT_COLOR_R) = light->color.r;
    light_node.append_attribute(ION_SAVE_LIGHT_COLOR_G) = light->color.g;
    light_node.append_attribute(ION_SAVE_LIGHT_COLOR_B) = light->color.b;
    light_node.append_attribute(ION_SAVE_LIGHT_TYPE) =
        static_cast<int>(light->type);
    light_node.append_attribute(ION_SAVE_LIGHT_INTENSITY) = light->intensity;
    light_node.append_attribute(ION_SAVE_LIGHT_RADIAL_FALLOFF) =
        light->radial_falloff;
    light_node.append_attribute(ION_SAVE_LIGHT_VOLUMETRIC_INTENSITY) =
        light->volumetric_intensity;
  }
  for (const auto &[entity_id, camera] : asset->GetComponentSet<Camera>()) {
    auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
    component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) =
        ION_SAVE_CAMERA_KEY;
    component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
    auto camera_node = component_node.append_child(ION_SAVE_CAMERA_KEY);
  }
  doc.save_file(path.c_str());
}
template <>
ION_API std::shared_ptr<World>
ion::res::LoadAsset<World>(std::filesystem::path path, bool is_hash) {
  SetProjectRoot(path.parent_path() / "assets");
  if (!std::filesystem::exists(path.parent_path() / "assets")) {
    std::filesystem::create_directory(path.parent_path() / "assets");
  }
  auto world = std::make_shared<World>(path);
  ProcessWorldManifest(world);
  if (is_hash) {
    internal::worlds.insert({path.filename().string(), world});
  } else {
    internal::worlds.insert(
        {ion::id::GenerateHashFromString(path.string()), world});
  }
  return world;
}
template <>
ION_API std::shared_ptr<Texture>
ion::res::LoadAsset<Texture>(std::filesystem::path source_path, bool is_hash) {
  std::string id;
  if (!is_hash) {
    if (!std::filesystem::exists(source_path)) {
      throw std::runtime_error(
          std::format("Texture does not exist: {}\n", source_path.string()));
    }
    id = ion::id::GenerateHashFromString(source_path.string());
    std::filesystem::copy_file(std::filesystem::absolute(source_path),
                               GetProjectRoot() / id,
                               std::filesystem::copy_options::update_existing);
  } else {
    id = source_path.filename().string();
  }

  std::filesystem::path imported_path = GetProjectRoot() / id;
  TextureInfo info{};
  info.data = stbi_load(imported_path.string().c_str(), &info.width,
                        &info.height, &info.nr_channels, 0);
  if (!info.data) {
    printf("Failed to load texture image: Path: %s, Reason: %s\n",
           imported_path.string().c_str(), stbi_failure_reason());
    return nullptr;
  }
  auto texture = std::make_shared<Texture>(imported_path, id);
  texture->texture = ion::render::ConfigureTexture(info);
  stbi_image_free(info.data);
  internal::textures.insert({id, texture});
  return texture;
}
template <>
ION_API std::shared_ptr<Shader>
ion::res::LoadAsset<Shader>(std::filesystem::path source_path, bool is_hash) {
  std::string id;
  if (!is_hash) {
    if (!std::filesystem::exists(source_path)) {
      throw std::runtime_error(std::format(
          "Shader directory does not exist: {}\n", source_path.string()));
    }
    if (!std::filesystem::exists(source_path / "vs.glsl") ||
        !std::filesystem::exists(source_path / "fs.glsl")) {
      throw std::runtime_error(
          std::format("Shader directory missing vs.glsl or fs.glsl: {}\n",
                      source_path.string()));
    }
    id = ion::id::GenerateHashFromString(source_path.string());
    auto shader_directory = GetProjectRoot() / id;
    std::filesystem::create_directory(shader_directory);
    std::filesystem::copy_file(source_path / "vs.glsl",
                               shader_directory / "vs.glsl",
                               std::filesystem::copy_options::update_existing);
    std::filesystem::copy_file(source_path / "fs.glsl",
                               shader_directory / "fs.glsl",
                               std::filesystem::copy_options::update_existing);
  } else {
    id = source_path.filename().string();
  }

  auto imported_path = GetProjectRoot() / id;
  auto shader = std::make_shared<Shader>(imported_path, id);
  internal::shaders.insert({id, shader});
  return shader;
}
template <>
ION_API std::shared_ptr<GPUData>
ion::res::LoadAsset<GPUData>(std::filesystem::path path, bool is_hash) {
  if (!is_hash) {
    if (!std::filesystem::exists(path)) {
      printf("GPUData manifest does not exist: %s\n", path.string().c_str());
      return nullptr;
    }
    auto id = ion::id::GenerateHashFromString(path.string());
    if (!std::filesystem::exists(GetProjectRoot() / id)) {
      printf("Copying asset from %s as %s\n",
             std::filesystem::absolute(path).string().c_str(), id.c_str());
      std::filesystem::copy_file(
          std::filesystem::absolute(path), GetProjectRoot() / id,
          std::filesystem::copy_options::update_existing);
    }
    path = GetProjectRoot() / id;
    auto gpu_data = std::make_shared<GPUData>(LoadGPUDataManifest(path), id);
    ion::render::ConfigureData(gpu_data);
    internal::gpu_datas.insert({id, gpu_data});
    return gpu_data;
  }
  auto gpu_data = std::make_shared<GPUData>(
      LoadGPUDataManifest(GetProjectRoot() / path), path.filename().string());
  ion::render::ConfigureData(gpu_data);
  internal::gpu_datas.insert({path.filename().string(), gpu_data});
  return gpu_data;
}
template <>
ION_API void ion::res::SaveAsset(std::filesystem::path path,
                                 std::shared_ptr<GPUData> asset) {
  auto doc = pugi::xml_document();
  auto root = doc.append_child("GPUData");
  root.append_attribute("element_enabled") = asset->element_enabled;
  for (const auto &pointer : asset->GetDescriptor().pointers) {
    auto pointer_node = root.append_child("AttributePointer");
    pointer_node.append_attribute("size") = pointer.size;
    std::string type_str;
    if (pointer.type == DataType::INT) {
      type_str = "INT";
    } else if (pointer.type == DataType::UNSIGNED_INT) {
      type_str = "UINT";
    } else if (pointer.type == DataType::FLOAT) {
      type_str = "FLOAT";
    }
    pointer_node.append_attribute("type") = type_str.c_str();
    pointer_node.append_attribute("normalized") = pointer.normalized;
    pointer_node.append_attribute("stride") =
        static_cast<unsigned long long>(pointer.stride);
    pointer_node.append_attribute("pointer") =
        reinterpret_cast<unsigned long long>(pointer.pointer);
  }
  auto vertices_node = root.append_child("vertices");
  for (const auto &vertex : asset->GetDescriptor().vertices) {
    auto vertex_node = vertices_node.append_child("vertex");
    vertex_node.append_attribute("val") = vertex;
  }
  auto indices_node = root.append_child("indices");
  for (const auto &index : asset->GetDescriptor().indices) {
    auto index_node = indices_node.append_child("index");
    index_node.append_attribute("val") = index;
  }
  doc.save_file(path.c_str());
}
