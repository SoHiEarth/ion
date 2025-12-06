#pragma once
#include "ion/exports.h"
#include "ion/world.h"
#include <memory>

class ION_API GameSystem {
public:
	void Update(std::shared_ptr<World>& world);
};