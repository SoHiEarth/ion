#include "development/gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>

static ImGuiContext* g_SharedImGuiContext = nullptr;
void* ion_gui_alloc(size_t size, void* user_data) {
	return malloc(size);
}

void ion_gui_free(void* ptr, void* user_data) {
	free(ptr);
}

void ion::gui::Init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	void* user_data = nullptr;
	ImGui::SetAllocatorFunctions(ion_gui_alloc, ion_gui_free, user_data);
	g_SharedImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(g_SharedImGuiContext);
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

void ion::gui::NewFrame() {
	ImGui::SetCurrentContext(g_SharedImGuiContext);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ion::gui::Render() {
	ImGui::SetCurrentContext(g_SharedImGuiContext);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ion::gui::Quit() {
	ImGui::SetCurrentContext(g_SharedImGuiContext);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(g_SharedImGuiContext);
	g_SharedImGuiContext = nullptr;
}

ImGuiContext* ion::gui::GetContext() {
	return g_SharedImGuiContext;
}