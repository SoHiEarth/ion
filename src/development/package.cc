#include "world.h"
#include "development/package.h"
#include <fstream>
#include <sstream>

void ion::dev::internal::WorldPathList::WriteToPath(const std::filesystem::path& path) {
	std::ofstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open world path list file for writing: " + path.string());
	}
	for (const auto& [world_id, world_info] : worlds) {
		file << world_id << "," << world_info.first.string() << "," << static_cast<int>(world_info.second) << "\n";
	}
}

static bool CheckWorldPath(const std::filesystem::path& path) {
	return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

void ion::dev::Packer::CreatePackaged(PackageData& package_data) {
	ion::dev::internal::WorldPathList world_list{};
	for (const auto& world_path : package_data.world_paths) {
		if (!CheckWorldPath(world_path.second)) {
			printf("World path %s does not exist or is not a file.\n", world_path.second.string().c_str());
		}
		else {
			// Get the relative path of world
			auto relative = std::filesystem::relative(world_path.second);
			world_list.worlds.insert({ world_path.first, {relative, ion::dev::internal::WorldLoadType::LOAD_ON_START} });
			printf("Added %s to world list\n", relative.c_str());
		}
	}
	
	try {
		world_list.WriteToPath(package_data.output_path.relative_path() / "worlds.lst");
	}
	catch (const std::exception& e) {
		printf("Failed to write world list: %s\n", e.what());
		return;
	}
}