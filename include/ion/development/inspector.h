#pragma once
#include <memory>
#include "ion/defaults.h"
#include "ion/world.h"

namespace ion::dev::ui {
	namespace internal {
		extern bool render_settings_open;
		extern bool asset_inspector_open;
	}
	void RenderInspector(std::shared_ptr<World>& world, Defaults& defaults);
}