#include "ion/game/game.h"
#include <GLFW/glfw3.h>
#include "ion/render.h"

void ion::game::Update(std::shared_ptr<World>& world) {
	for (auto& [entity, marker] : world->GetMarkers()) {
		if (marker == "player") {
			auto transform = world->GetComponent<Transform>(entity);
			auto window = ion::render::GetWindow();
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				transform->position.y += 0.1f;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				transform->position.y -= 0.1f;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				transform->position.x -= 0.1f;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				transform->position.x += 0.1f;
			}
		}
	}
}