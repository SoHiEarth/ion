#include "ion/development/write_to_disk.h"
#include "ion/world.h"
#include "ion/texture.h"
#include "ion/shader.h"
#include <fstream>
#include <sstream>
#include <pugixml.hpp>
#include "ion/save_keys.h"

void ion::dev::Writer::WriteToDisk(std::filesystem::path path, std::shared_ptr<World> world) {
	auto doc = pugi::xml_document();
	auto declaration = doc.append_child(ION_SAVE_METADATA);
	declaration.append_attribute(ION_SAVE_VERSION) = ION_BUILD_VERSION;
	auto root = doc.append_child(ION_SAVE_WORLD);
	for (const auto& [entity_id, marker_name] : world->GetMarkers()) {
		auto marker_node = root.append_child(ION_SAVE_MARKER_KEY);
		marker_node.append_attribute(ION_SAVE_MARKER_ID) = entity_id;
		marker_node.append_attribute(ION_SAVE_MARKER_VAL) = marker_name.c_str();
	}
	for (const auto& [entity_id, transform] : world->GetComponentSet<Transform>()) {
		auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
		component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) = ION_SAVE_TRANSFORM_KEY;
		component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
		auto transform_node = component_node.append_child(ION_SAVE_TRANSFORM_KEY);
		transform_node.append_attribute(ION_SAVE_TRANSFORM_POS_X) = transform->position.x;
		transform_node.append_attribute(ION_SAVE_TRANSFORM_POS_Y) = transform->position.y;
		transform_node.append_attribute(ION_SAVE_TRANSFORM_ROTATION) = transform->rotation;
		transform_node.append_attribute(ION_SAVE_TRANSFORM_SCALE_X) = transform->scale.x;
		transform_node.append_attribute(ION_SAVE_TRANSFORM_SCALE_Y) = transform->scale.y;
	}
	for (const auto& [entity_id, renderable] : world->GetComponentSet<Renderable>()) {
		auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
		component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) = ION_SAVE_RENDERABLE_KEY;
		component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
		auto renderable_node = component_node.append_child(ION_SAVE_RENDERABLE_KEY);
		renderable_node.append_attribute(ION_SAVE_RENDERABLE_COLOR) = renderable->color->GetID().c_str();
		renderable_node.append_attribute(ION_SAVE_RENDERABLE_NORMAL) = renderable->normal->GetID().c_str();
		renderable_node.append_attribute(ION_SAVE_RENDERABLE_SHADER) = renderable->shader->GetID().c_str();
	}
	for (const auto& [entity_id, light] : world->GetComponentSet<Light>()) {
		auto component_node = root.append_child(ION_SAVE_COMPONENT_KEY);
		component_node.append_attribute(ION_SAVE_COMPONENT_TYPE) = ION_SAVE_LIGHT_KEY;
		component_node.append_attribute(ION_SAVE_ENTITY_ID) = entity_id;
		auto light_node = component_node.append_child(ION_SAVE_LIGHT_KEY);
		light_node.append_attribute(ION_SAVE_LIGHT_COLOR_R) = light->color.r;
		light_node.append_attribute(ION_SAVE_LIGHT_COLOR_G) = light->color.g;
		light_node.append_attribute(ION_SAVE_LIGHT_COLOR_B) = light->color.b;
		light_node.append_attribute(ION_SAVE_LIGHT_TYPE) = static_cast<int>(light->type);
		light_node.append_attribute(ION_SAVE_LIGHT_INTENSITY) = light->intensity;
		light_node.append_attribute(ION_SAVE_LIGHT_RADIAL_FALLOFF) = light->radial_falloff;
		light_node.append_attribute(ION_SAVE_LIGHT_VOLUMETRIC_INTENSITY) = light->volumetric_intensity;
	}
	doc.save_file(path.c_str());
}