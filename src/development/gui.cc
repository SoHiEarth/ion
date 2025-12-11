#include "ion/development/gui.h"
#include "ion/render/api.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

static ImGuiContext *g_SharedImGuiContext = nullptr;
void *ion_gui_alloc(size_t size, void *user_data) { return malloc(size); }

void ion_gui_free(void *ptr, void *user_data) { free(ptr); }

constexpr float GUI_ROUNDING_MORE = 8.0F;
constexpr float GUI_ROUNDING_LESS = 6.0F;

void ion::gui::Init(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  void *user_data = nullptr;
  ImGui::SetAllocatorFunctions(ion_gui_alloc, ion_gui_free, user_data);
  g_SharedImGuiContext = ImGui::CreateContext();
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGuiIO &imgui_io = ImGui::GetIO();
  imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  imgui_io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = GUI_ROUNDING_MORE;
  style.ChildRounding = GUI_ROUNDING_MORE;
  style.FrameRounding = GUI_ROUNDING_LESS;
  style.PopupRounding = GUI_ROUNDING_LESS;
  style.ScrollbarRounding = GUI_ROUNDING_LESS;
  style.GrabRounding = GUI_ROUNDING_LESS;
  style.TabRounding = GUI_ROUNDING_LESS;
  ImGui_ImplGlfw_InitForVulkan(window, true);
  auto info = ImGui_ImplVulkan_InitInfo{};
  info.Instance = ion::render::api::internal::instance;
  ImGui_ImplVulkan_Init(&info);
}

void ion::gui::NewFrame() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui_ImplGlfw_NewFrame();
  ImGui_ImplVulkan_NewFrame();
  ImGui::NewFrame();
}

void ion::gui::Render() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), nullptr);
}

void ion::gui::Quit() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(g_SharedImGuiContext);
  g_SharedImGuiContext = nullptr;
}

ImGuiContext *ion::gui::GetContext() { return g_SharedImGuiContext; }
