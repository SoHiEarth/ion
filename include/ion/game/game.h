#pragma once
#include "ion/exports.h"
#include "ion/world.h"
#include <memory>

namespace ion::game {
	ION_API void Update(std::shared_ptr<World>& world);
};