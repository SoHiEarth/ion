#pragma once
#include "exports.h"
#include "world.h"
#include <memory>

class ION_API GameSystem {
public:
	void Update(std::shared_ptr<World>& world);
};