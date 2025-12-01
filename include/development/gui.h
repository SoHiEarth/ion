#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "exports.h"
#define ION_GUI_PREP_CONTEXT() ImGui::SetCurrentContext(ion::gui::GetContext());

namespace ion {
	namespace gui {
		ION_API void Init(GLFWwindow* window);
		ION_API void NewFrame();
		ION_API void Render();
		ION_API void Quit();
		ION_API ImGuiContext* GetContext();
	} // namespace gui
} // namespace ion