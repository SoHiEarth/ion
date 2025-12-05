#pragma once
#include "exports.h"
#include "world.h"
#include <memory>
#include <string>

namespace ion {
	namespace dev {
		class ION_API Writer {
		public:
			static void WriteToDisk(std::string_view filepath, std::shared_ptr<World> world);
		};
	} // namespace dev
} // namespace ion