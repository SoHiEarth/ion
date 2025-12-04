#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <functional>
#include <memory>
#include "exports.h"
#define ION_GUI_PREP_CONTEXT() ImGui::SetCurrentContext(ion::gui::GetContext());

namespace ion {
	namespace gui {
		struct Button {
			std::function<void()> on_click;
		};

		ION_API void Init(GLFWwindow* window);
		ION_API void NewFrame();
		ION_API void Render();
		ION_API void Quit();
		ION_API ImGuiContext* GetContext();
	} // namespace gui
} // namespace ion