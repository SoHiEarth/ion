#pragma once
#include "exports.h"
#include <memory>

class ION_API Defaults {
public:
	std::shared_ptr<struct TexturePack> default_textures;
	std::shared_ptr<struct Shader> default_shader;
	std::shared_ptr<struct GPUData> default_data;
};