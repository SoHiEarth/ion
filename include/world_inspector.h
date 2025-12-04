#pragma once
#include <memory>
#include "exports.h"

class ION_API Defaults {
public:
	std::shared_ptr<class TexturePack> default_textures;
	std::shared_ptr<class Shader> default_shader;
	std::shared_ptr<class GPUData> default_data;
};

ION_API void WorldInspector(std::shared_ptr<struct World> world, Defaults& defaults);