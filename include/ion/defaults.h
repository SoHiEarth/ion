#pragma once
#include "exports.h"
#include <memory>

class Defaults {
public:
	std::shared_ptr<struct Texture> default_color;
	std::shared_ptr<struct Texture> default_normal;
	std::shared_ptr<struct Shader> default_shader;
	std::shared_ptr<struct GPUData> default_data;
	Defaults();
};