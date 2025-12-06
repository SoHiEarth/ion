#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include <string>
#include "exports.h"

struct Texture;
struct Shader;
struct GPUData;

enum class ProjectionMode : std::uint8_t { PERSPECTIVE = 0, ORTHOGRAPHIC = 1 };
enum class LightType : std::uint8_t { GLOBAL = 0, POINT = 1 };
struct ION_API Transform {
  glm::vec2 position = glm::vec2(0.0f);
  int layer = 0;
  glm::vec2 scale = glm::vec2(1.0f);
  float rotation = 0.0f;
};

struct ION_API PhysicsBody {
  b2BodyId body_id{};
  bool enabled = false;
};

struct ION_API Renderable {
  std::shared_ptr<Texture> color = nullptr;
  std::shared_ptr<Texture> normal = nullptr;
  std::shared_ptr<Shader> shader = nullptr;
  std::shared_ptr<GPUData> data = nullptr;
};

struct ION_API Light {
	LightType type = LightType::POINT;
  float intensity = 1.0f;
  float radial_falloff = 1.0f;
	float volumetric_intensity = 0.0f;
  glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct ION_API Camera {
  glm::vec2 position = glm::vec2(0.0f, 0.0f);
  ProjectionMode mode = ProjectionMode::PERSPECTIVE;
};


struct ION_API Script {
  std::string path = "";
  std::string module_name = "main";
  std::map<std::string, std::string> parameters = {};
};