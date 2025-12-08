#pragma once
#include <memory>
#include <map>
#include <string>
#include "ion/defaults.h"
#include "ion/world.h"

namespace ion::dev::ui {
	namespace internal {
		extern std::map<std::string, bool> inspector_state;
	}
	void RenderInspector(std::shared_ptr<World>& world, Defaults& defaults);
}