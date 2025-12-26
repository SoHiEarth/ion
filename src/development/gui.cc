#include "ion/development/gui.h"
#include "ion/render/api.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>

static ImGuiContext* g_SharedImGuiContext = nullptr;
static void* ion_gui_alloc(size_t size, void* user_data) { return malloc(size); }
static void ion_gui_free(void* ptr, void* user_data) { free(ptr); }

constexpr float GUI_ROUNDING_MORE = 8.0F;
constexpr float GUI_ROUNDING_LESS = 6.0F;

void ion::gui::Init(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  void* user_data = nullptr;
  ImGui::SetAllocatorFunctions(ion_gui_alloc, ion_gui_free, user_data);
  g_SharedImGuiContext = ImGui::CreateContext();
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGuiIO& imgui_io = ImGui::GetIO();
  imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  imgui_io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = GUI_ROUNDING_MORE;
  style.ChildRounding = GUI_ROUNDING_MORE;
  style.FrameRounding = GUI_ROUNDING_LESS;
  style.PopupRounding = GUI_ROUNDING_LESS;
  style.ScrollbarRounding = GUI_ROUNDING_LESS;
  style.GrabRounding = GUI_ROUNDING_LESS;
  style.TabRounding = GUI_ROUNDING_LESS;

  std::vector<VkDescriptorPoolSize> pool_sizes = {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
  };
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  if (vkCreateDescriptorPool(ion::render::api::internal::device, &pool_info, nullptr, &ion::render::api::internal::descriptor_pool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create ImGui descriptor pool!");
  }

  ImGui_ImplGlfw_InitForVulkan(window, true);
  auto info = ImGui_ImplVulkan_InitInfo{};
  info.Instance = ion::render::api::internal::instance;
  info.PhysicalDevice = ion::render::api::internal::physical_device;
  info.Device = ion::render::api::internal::device;
  auto queue_family = ion::render::api::internal::FindQueueFamilies(ion::render::api::internal::physical_device);
  info.QueueFamily = queue_family.graphics_family.value();
  info.Queue = ion::render::api::internal::graphics_queue;
  info.DescriptorPool = ion::render::api::internal::descriptor_pool;
  info.MinImageCount = 2;
  info.ImageCount = ion::render::api::internal::swapchain_images.size();
  info.Allocator = nullptr;
  info.RenderPass = ion::render::api::internal::render_pass;
  info.PipelineCache = VK_NULL_HANDLE;
  info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&info);
  ImGui_ImplVulkan_CreateFontsTexture();
}

void ion::gui::NewFrame() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ion::gui::Render() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui::Render();
}

void ion::gui::Quit() {
  ImGui::SetCurrentContext(g_SharedImGuiContext);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(g_SharedImGuiContext);
  g_SharedImGuiContext = nullptr;
}

ImGuiContext *ion::gui::GetContext() { return g_SharedImGuiContext; }
