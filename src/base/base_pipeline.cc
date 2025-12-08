#include "ion/base_pipeline.h"
#include "ion/assets.h"
#include "ion/render.h"
#include "ion/shader.h"

BasePipeline::BasePipeline() {
  color_buffer = ion::render::CreateFramebuffer(
      FramebufferInfo{.recreate_on_resize = true, .name = "Color"});
  normal_buffer = ion::render::CreateFramebuffer(
      FramebufferInfo{.recreate_on_resize = true, .name = "Normal"});
  shaded = ion::render::CreateFramebuffer(
      FramebufferInfo{.recreate_on_resize = true, .name = "Shaded"});
  bloom_buffer = ion::render::CreateFramebuffer(
      FramebufferInfo{.recreate_on_resize = true, .name = "Bloom"});
  bloom_buffer_2 = ion::render::CreateFramebuffer(
      FramebufferInfo{.recreate_on_resize = true, .name = "Bloom 2"});
  deferred_shader =
      ion::res::LoadAsset<Shader>("assets/deferred_shader", false);
  screen_shader = ion::res::LoadAsset<Shader>("assets/screen_shader", false);
  bloom_shader = ion::res::LoadAsset<Shader>("assets/bloom_shader", false);
  bloom_blur_shader =
      ion::res::LoadAsset<Shader>("assets/bloom_blur_shader", false);
  combine_shader =
      ion::res::LoadAsset<Shader>("assets/bloom_combine_shader", false);
  tonemap_shader = ion::res::LoadAsset<Shader>("assets/tonemap_shader", false);

  screen_data = ion::res::LoadAsset<GPUData>("assets/screen_quad", false);
}

void BasePipeline::Render(std::shared_ptr<World> world,
                          const PipelineSettings &settings) {
  ion::render::BindFramebuffer(color_buffer);
  ion::render::Clear();
  ion::render::DrawWorld(world, RENDER_PASS_COLOR);
  ion::render::BindFramebuffer(normal_buffer);
  ion::render::Clear();
  ion::render::DrawWorld(world, RENDER_PASS_NORMAL);

  ion::render::BindFramebuffer(shaded);
  ion::render::Clear();
  ion::render::Render(color_buffer, normal_buffer, screen_data, deferred_shader,
                      world);
  ion::render::UnbindFramebuffer();

  if (settings.bloom_enable) {
    ion::render::BindFramebuffer(bloom_buffer);
    ion::render::Clear();
    ion::render::UseShader(bloom_shader);
    ion::render::RunPass(shaded, bloom_buffer, bloom_shader, screen_data);

    ion::render::UseShader(bloom_blur_shader);
    bool horizontal = true;

    for (int i = 0; i < settings.bloom_strength; i++) {
      auto &source = horizontal ? bloom_buffer : bloom_buffer_2;
      auto &target = horizontal ? bloom_buffer_2 : bloom_buffer;
      ion::render::BindFramebuffer(target);
      ion::render::Clear();
      bloom_blur_shader->SetUniform("horizontal", (int)horizontal);
      ion::render::RunPass(source, target, bloom_blur_shader, screen_data);
      horizontal = !horizontal;
    }

    ion::render::UseShader(combine_shader);
    ion::render::BindTexture(shaded, 1);
    combine_shader->SetUniform("ION_PASS_FRAMEBUFFER", 1);
    ion::render::RunPass(bloom_buffer, shaded, combine_shader, screen_data);
  }

  ion::render::DrawFramebuffer(shaded, screen_shader, screen_data);
}
