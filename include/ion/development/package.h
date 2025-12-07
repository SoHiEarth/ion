#pragma once
#include "ion/world.h"
#include <string>
#include <memory>
#include <filesystem>
#include <map>

namespace ion {
	namespace dev {		
		namespace internal {
			struct PackInfo {
				// World ID (in order of execution) to pair of {path, load type}
				std::map<int, std::filesystem::path> worlds;
				void Write(const std::filesystem::path& path);
			};
		}

		struct PackageData {
			std::map<int, std::filesystem::path> world_paths;
			std::filesystem::path output_path;
		};

		class Packer {
		public:
			static bool CreatePackaged(PackageData& package_data);
		};
	}
}
