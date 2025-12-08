#pragma once
#include <memory>
struct Framebuffer;
struct Shader;
struct GPUData;
struct World;

struct PipelineSettings {
  bool bloom_enable = false;
  int bloom_strength = 10;
};

struct BasePipeline {
  std::shared_ptr<Framebuffer> color_buffer;
  std::shared_ptr<Framebuffer> normal_buffer;
  std::shared_ptr<Framebuffer> shaded;
  std::shared_ptr<Framebuffer> bloom_buffer;
  std::shared_ptr<Framebuffer> bloom_buffer_2;

  std::shared_ptr<Shader> deferred_shader;
  std::shared_ptr<Shader> screen_shader;
  std::shared_ptr<Shader> bloom_shader;
  std::shared_ptr<Shader> bloom_blur_shader;
  std::shared_ptr<Shader> combine_shader;
  std::shared_ptr<Shader> tonemap_shader;

  std::shared_ptr<GPUData> screen_data;

  void Render(std::shared_ptr<World> world, const PipelineSettings &settings);
  BasePipeline();
};