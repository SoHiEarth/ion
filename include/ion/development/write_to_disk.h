#pragma once
#include "ion/exports.h"
#include "ion/world.h"
#include <memory>
#include <filesystem>

namespace ion {
	namespace dev {
		class ION_API Writer {
		public:
			static void WriteToDisk(std::filesystem::path filepath, std::shared_ptr<World> world);
		};
	} // namespace dev
} // namespace ion