#include "ion/assets.h"
#include "ion/context.h"
#include "ion/physics.h"
#include "ion/texture.h"
#include "ion/render.h"
#include "ion/world.h"
#include "ion/script.h"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <vector>
#include <format>
#include "ion/shader.h"
#include "ion/development/inspector.h"
#include "ion/development/gui.h"
#include "ion/game/game.h"

std::vector<float> vertices = {
   0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   0.5f, -0.5f,  0.0f,  1.0f,  1.0f,
  -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,
  -0.5f,  0.5f,  0.0f,  0.0f,  0.0f
},
screen_vertices = {
   1.0f,  1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,  0.0f,
  -1.0f, -1.0f,  0.0f,  0.0f,
  -1.0f,  1.0f,  0.0f,  1.0f
};

GameSystem game_system;

std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
bool bloom_enable = true;

int main(int argc, char **argv) {
	if (!ion::GetSystem<AssetSystem>().CheckApplicationStructure()) {
    printf("Invalid application structure. Exiting.\n");
    return -1;
	}
  auto world_path = tinyfd_openFileDialog("Open World", nullptr, 0, nullptr, nullptr, false);
  if (!world_path) {
    tinyfd_messageBox("Error", "No world selected. Exiting.", "ok", "error", 1);
    return -1;
  }
  auto world = ion::GetSystem<AssetSystem>().LoadAsset<World>(world_path);
  ion::GetSystem<RenderSystem>().Init();
  ion::gui::Init(ion::GetSystem<RenderSystem>().GetWindow());
  ion::GetSystem<PhysicsSystem>().Init();
  ion::GetSystem<ScriptSystem>().Init();

  AttributePointer position_pointer {
    .size = 3,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void *)0
  };
  AttributePointer texture_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void *)(3 * sizeof(float))
  };
  DataDescriptor data_desc {
    .pointers = {position_pointer, texture_pointer},
    .element_enabled = true,
    .vertices = vertices,
    .indices = indices
  };

  AttributePointer screen_position_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)0
	};
  AttributePointer screen_texture_pointer {
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void *)(2 * sizeof(float))
	};
  DataDescriptor screen_data_desc {
    .pointers = {screen_position_pointer, screen_texture_pointer},
    .element_enabled = true,
    .vertices = screen_vertices,
    .indices = indices
  };
  auto screen_data = ion::GetSystem<RenderSystem>().CreateData(screen_data_desc);

  auto framebuffer_info = FramebufferInfo{
      .recreate_on_resize = true
  };

  framebuffer_info.name = "Color";
  auto color_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Normal";
  auto normal_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Shaded";
  auto shaded = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom";
  auto bloom_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom 2";
  auto bloom_buffer2 = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
	framebuffer_info.name = "Tonemapped";
	auto tonemap_buffer = ion::GetSystem<RenderSystem>().CreateFramebuffer(framebuffer_info);
  auto& final_framebuffer = shaded;

  auto deferred_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/deferred_shader");
  auto screen_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/screen_shader");
  auto bloom_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_shader");
  auto bloom_blur_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_blur_shader");
  auto combine_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/bloom_combine_shader");
  auto tonemap_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/tonemap_shader");

  auto defaults = Defaults{
    .default_color = ion::GetSystem<AssetSystem>().LoadAsset<Texture>("assets/test_sprite/color.png"),
    .default_normal = ion::GetSystem<AssetSystem>().LoadAsset<Texture>("assets/test_sprite/normal.png"),
    .default_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/texture_shader"),
    .default_data = ion::GetSystem<RenderSystem>().CreateData(data_desc)
	};

  auto camera_entity = world->CreateEntity();
  world->NewComponent<Camera>(camera_entity);
  auto entity = world->CreateEntity();
  auto renderable = world->NewComponent<Renderable>(entity);
	renderable->color = defaults.default_color;
  renderable->normal = defaults.default_normal;
	renderable->shader = defaults.default_shader;
	renderable->data = defaults.default_data;

  auto light_entity = world->CreateEntity();
	auto light = world->NewComponent<Light>(light_entity);
	light->type = LightType::GLOBAL;
	light->intensity = 1.0f;
	light->radial_falloff = 1.0f;
	light->color = glm::vec3(1.0f, 1.0f, 1.0f);

  while (!glfwWindowShouldClose(ion::GetSystem<RenderSystem>().GetWindow())) {
    glfwPollEvents();
		ion::gui::NewFrame();
    ion::GetSystem<ScriptSystem>().Update(world);
    ion::GetSystem<PhysicsSystem>().Update();
		game_system.Update(world);

    ION_GUI_PREP_CONTEXT();
    {
      ImGui::Begin("Framebuffers");
			ImGui::Checkbox("Enable Bloom", &bloom_enable);
      for (auto& [buffer, name] : ion::GetSystem<RenderSystem>().GetFramebuffers()) {
        ImGui::PushID(buffer->framebuffer);
        ImGui::Image(buffer->colorbuffer,
          ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::TextUnformatted(name.c_str());
        ImGui::PopID();
      }

      ImGui::End();
    }

    WorldInspector(world, defaults);
    AssetInspector();
    
    ion::GetSystem<RenderSystem>().BindFramebuffer(color_buffer);
    ion::GetSystem<RenderSystem>().Clear();
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_COLOR);
    ion::GetSystem<RenderSystem>().BindFramebuffer(normal_buffer);
    ion::GetSystem<RenderSystem>().Clear();
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_NORMAL);

    ion::GetSystem<RenderSystem>().BindFramebuffer(shaded);
    ion::GetSystem<RenderSystem>().Clear();
    ion::GetSystem<RenderSystem>().Render(color_buffer, normal_buffer, screen_data, deferred_shader, world);
    ion::GetSystem<RenderSystem>().UnbindFramebuffer();

    if (bloom_enable) {
      ion::GetSystem<RenderSystem>().BindFramebuffer(bloom_buffer);
      ion::GetSystem<RenderSystem>().Clear();
      ion::GetSystem<RenderSystem>().UseShader(bloom_shader);
      ion::GetSystem<RenderSystem>().RunPass(shaded, bloom_buffer, bloom_shader, screen_data);

      ion::GetSystem<RenderSystem>().UseShader(bloom_blur_shader);
      bool horizontal = true;
      int blur_amount = 10;

      for (int i = 0; i < blur_amount; i++) {
        auto& source = horizontal ? bloom_buffer : bloom_buffer2;
        auto& target = horizontal ? bloom_buffer2 : bloom_buffer;
        ion::GetSystem<RenderSystem>().BindFramebuffer(target);
        ion::GetSystem<RenderSystem>().Clear();
        bloom_blur_shader->SetUniform("horizontal", (int)horizontal);
        ion::GetSystem<RenderSystem>().RunPass(source, target, bloom_blur_shader, screen_data);
        horizontal = !horizontal;
      }

      ion::GetSystem<RenderSystem>().UseShader(combine_shader);
      ion::GetSystem<RenderSystem>().BindTexture(shaded, 1);
      combine_shader->SetUniform("ION_PASS_FRAMEBUFFER", 1);
      ion::GetSystem<RenderSystem>().RunPass(bloom_buffer, shaded, combine_shader, screen_data);
    }

    //Context::Get().render_sys.UseShader(tonemap_shader);
    //Context::Get().render_sys.RunPass(tonemap_buffer, shaded, tonemap_shader, screen_data);

    ion::GetSystem<RenderSystem>().DrawFramebuffer(final_framebuffer, screen_shader, screen_data);
    ion::gui::Render();
    ion::GetSystem<RenderSystem>().Present();
  }
  ion::GetSystem<PhysicsSystem>().Quit();
  ion::GetSystem<RenderSystem>().DestroyData(screen_data);
  ion::GetSystem<RenderSystem>().DestroyData(defaults.default_data);
  ion::GetSystem<RenderSystem>().DestroyShader(screen_shader);
  ion::GetSystem<RenderSystem>().DestroyShader(deferred_shader);
  ion::GetSystem<RenderSystem>().DestroyShader(defaults.default_shader);
  ion::GetSystem<ScriptSystem>().Quit();
  ion::GetSystem<RenderSystem>().Quit();
  ion::gui::Quit();
  return 0;
}