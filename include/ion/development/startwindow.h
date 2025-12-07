#pragma once
#include <memory>
#include "ion/world.h"

namespace ion {
	namespace gui {
		enum class STARTWINDOW_EXCEPTION {
			CANCELLED = 1
		};

		enum class STARTWINDOW_LATEFUNC {
			UNSPECIFIED = 0,
			NEW_WORLD_DIALOG = 1,
			OPEN_WORLD_DIALOG = 2
		};

		std::shared_ptr<World> StartWindow();
	}
}