#pragma once
#include "world.h"
#include <string>
#include <memory>
#include <filesystem>
#include <map>

namespace ion {
	namespace dev {
		namespace internal {
			enum class WorldLoadType {
				LOAD_ON_START,
				LOAD_ON_DEMAND
			};

			struct ION_API WorldPathList {
				// Map Info: World ID (in order of execution) to pair of {path, load type}
				std::map<int, std::pair<std::filesystem::path, WorldLoadType>> worlds;
				void WriteToPath(const std::filesystem::path& path);
			};
		}
		struct ION_API PackageData {
			std::map<int, std::filesystem::path> world_paths;
			std::filesystem::path output_path;
		};

		class ION_API Packer {
		public:
			static void CreatePackaged(PackageData& package_data);
		};
	}
}
