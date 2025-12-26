#include "ion/render/api.h"
#include <fstream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

static std::vector<char> ReadFile(std::string_view path) {
  auto file = std::ifstream(path.data(), std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file");
  }
  size_t file_size = (size_t)file.tellg();
  auto buffer = std::vector<char>(file_size);
  file.seekg(0);
  file.read(buffer.data(), file_size);
  file.close();
  return buffer;
}

VkShaderModule CreateShaderModule(const std::vector<char> &code) {
  auto info = VkShaderModuleCreateInfo{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = code.size();
  info.pCode = reinterpret_cast<const uint32_t *>(code.data());
  auto module = VkShaderModule{};
  if (vkCreateShaderModule(ion::render::api::internal::device, &info, nullptr,
                           &module) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module");
  }
  return module;
}

void ion::render::api::CreatePipeline() {
  auto vertex_module = CreateShaderModule(ReadFile("assets/test/vs.spv"));
  auto fragment_module = CreateShaderModule(ReadFile("assets/test/fs.spv"));

  auto vertex_info = VkPipelineShaderStageCreateInfo{};
  vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertex_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_info.module = vertex_module;
  vertex_info.pName = "main";

  auto fragment_info = VkPipelineShaderStageCreateInfo{};
  fragment_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragment_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_info.module = fragment_module;
  fragment_info.pName = "main";

  VkPipelineShaderStageCreateInfo stages[] = {vertex_info, fragment_info};

  auto input_info = VkPipelineVertexInputStateCreateInfo{};
  input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  input_info.vertexBindingDescriptionCount = 0;
  input_info.pVertexBindingDescriptions = nullptr;
  input_info.vertexAttributeDescriptionCount = 0;
  input_info.pVertexAttributeDescriptions = nullptr;

  auto input_assembly = VkPipelineInputAssemblyStateCreateInfo{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  auto viewport = VkViewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(internal::swapchain_extent.width);
  viewport.height = static_cast<float>(internal::swapchain_extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  auto scissor = VkRect2D{};
  scissor.offset = {0, 0};
  scissor.extent = internal::swapchain_extent;

  auto dynamic_states = std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR};
  auto dynamic_state = VkPipelineDynamicStateCreateInfo{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  auto viewport_state = VkPipelineViewportStateCreateInfo{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  auto rasterizer = VkPipelineRasterizationStateCreateInfo{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  auto multisample = VkPipelineMultisampleStateCreateInfo{};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.sampleShadingEnable = VK_FALSE;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.minSampleShading = 1.0f;
  multisample.pSampleMask = nullptr;
  multisample.alphaToCoverageEnable = VK_FALSE;
  multisample.alphaToOneEnable = VK_FALSE;

  auto color_blend_attachment = VkPipelineColorBlendAttachmentState{};
  color_blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_TRUE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  auto color_blending = VkPipelineColorBlendStateCreateInfo{};
  color_blending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;

  auto pipeline_layout_info = VkPipelineLayoutCreateInfo{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(internal::device, &pipeline_layout_info, nullptr,
                             &internal::pipeline_layout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  auto pipeline_info = VkGraphicsPipelineCreateInfo{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = stages;
  pipeline_info.pVertexInputState = &input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisample;
  pipeline_info.pDepthStencilState = nullptr;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state;
  pipeline_info.layout = internal::pipeline_layout;
  pipeline_info.renderPass = internal::render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(internal::device, VK_NULL_HANDLE, 1,
                                &pipeline_info, nullptr,
                                &internal::graphics_pipeline) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline");
  }

  vkDestroyShaderModule(internal::device, vertex_module, nullptr);
  vkDestroyShaderModule(internal::device, fragment_module, nullptr);
}
