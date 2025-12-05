#include "development/write_to_disk.h"
#include "world.h"
#include <fstream>
#include <sstream>

void ion::dev::Writer::WriteToDisk(std::filesystem::path path, std::shared_ptr<World> world) {
	std::ofstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open world file for writing: " + path.string());
	}
	// Write markers
	file << "[MARKER]\n";
	for (const auto& [entity_id, marker_name] : world->GetMarkers()) {
		file << entity_id << "=" << marker_name << "\n";
	}
	file << "[END_MARKER]\n";
	// Write components
	file << "[COMPONENTS]\n";
	// Transforms
	for (const auto& [entity_id, transform] : world->GetComponentSet<Transform>()) {
		file << "Transform|" << entity_id << "|"
			<< transform->position.x << "," << transform->position.y << "|"
			<< transform->layer << "|"
			<< transform->scale.x << "," << transform->scale.y << "|"
			<< transform->rotation << "\n";
	}
	file << "[END_COMPONENTS]\n";
	file.close();
}