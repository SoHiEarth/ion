#include "ion/development/package.h"
#include "ion/world.h"
#include <fstream>
#include <sstream>
#include <pugixml.hpp>
#define TOOLS_RUNNER_DIRECTORY ""
#define TOOLS_RUNNER_LOCATION "ion-run.exe"

void ion::dev::internal::PackInfo::Write(const std::filesystem::path &path) {
	pugi::xml_document doc;
	auto root = doc.append_child("worlds");
  for (const auto& [id, world_path] : worlds) {
    auto world_node = root.append_child("world");
    world_node.append_attribute("id") = id;
    world_node.append_attribute("path") = ("assets" / world_path.filename()).string().c_str();
	}
	doc.save_file(path.string().c_str());
}

void PrepareDirectory(std::filesystem::path path) {
  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directory(path);
  }
  if (!std::filesystem::exists(path / "assets")) {
    std::filesystem::create_directory(path / "assets");
  }
}

static bool CheckWorldPath(const std::filesystem::path &path) {
  return std::filesystem::exists(path) &&
         std::filesystem::is_regular_file(path);
}

std::map<int, std::filesystem::path>
CheckWorlds(std::filesystem::path output,
            std::map<int, std::filesystem::path> world_infos) {
  std::map<int, std::filesystem::path> valid_worlds;
  for (const auto &[id, path] : world_infos) {
    if (!CheckWorldPath(path)) {
      printf("World path %s does not exist or is not a file.\n",
             path.string().c_str());
    } else {
      printf("World path %s added.\n", path.string().c_str());
      valid_worlds.insert({id, path});
    }
  }
  return valid_worlds;
}

void CopyAssets(std::filesystem::path world_path,
                std::filesystem::path output_path) {
  // Copy world-specific assets (world_path/assets) into global assets folder
  // (output_path/assets)
  std::filesystem::path world_assets = world_path.parent_path() / "assets";
  std::filesystem::path output_assets = output_path / "assets";
  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(world_assets)) {
      if (entry.is_regular_file()) {
        std::filesystem::path relative_path =
            std::filesystem::relative(entry.path(), world_assets);
        std::filesystem::path target_path = output_assets / relative_path;

        std::filesystem::create_directories(target_path.parent_path());
        std::filesystem::copy_file(
            entry.path(), target_path,
            std::filesystem::copy_options::overwrite_existing);
        printf("Copied asset %s to %s\n",
               entry.path().filename().string().c_str(),
               target_path.string().c_str());
      }
    }
  } catch (const std::exception &e) {
    printf("Failed to copy assets from %s: %s\n", world_assets.string().c_str(),
           e.what());
  }
}

void CopyRunner(std::filesystem::path output) {
  if (!std::filesystem::exists(TOOLS_RUNNER_LOCATION)) {
    printf("Runner not found at %s\n", TOOLS_RUNNER_LOCATION);
    return;
  }
  std::filesystem::copy_file(TOOLS_RUNNER_LOCATION, output / "ion-run.exe",
                             std::filesystem::copy_options::overwrite_existing);
  std::filesystem::directory_iterator dll_iter(std::filesystem::absolute(std::filesystem::path(TOOLS_RUNNER_LOCATION)).parent_path());
  for (const auto& dll_entry : dll_iter) {
    if (dll_entry.path().extension() == ".dll") {
      std::filesystem::copy_file(
        dll_entry.path(), output / dll_entry.path().filename(),
        std::filesystem::copy_options::overwrite_existing);
      printf("Copied %s to %s\n", dll_entry.path().filename().string().c_str(),
        std::filesystem::path(TOOLS_RUNNER_LOCATION).parent_path());
    }
  }
}

bool ion::dev::Packer::CreatePackaged(PackageData &data) {
  PrepareDirectory(data.output_path);
  ion::dev::internal::PackInfo info{};
  info.worlds = CheckWorlds(data.output_path, data.world_paths);
  for (auto &[id, path] : info.worlds) {
    std::filesystem::copy_file(
        path, data.output_path / "assets" / path.filename(),
        std::filesystem::copy_options::overwrite_existing);
    CopyAssets(path, data.output_path);
  }
  CopyRunner(data.output_path);

  try {
    info.Write(data.output_path / "world_list.cfg");
  } catch (const std::exception &e) {
    printf("Failed to write world list: %s\n", e.what());
    return false;
  }
  return true;
}