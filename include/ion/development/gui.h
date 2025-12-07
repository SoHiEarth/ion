#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <memory>
#include "ion/exports.h"
#define ION_GUI_PREP_CONTEXT() ImGui::SetCurrentContext(ion::gui::GetContext());

namespace ion {
	namespace gui {
		void Init(GLFWwindow* window);
		void NewFrame();
		void Render();
		void Quit();
		ImGuiContext* GetContext();
	} // namespace gui
} // namespace ion