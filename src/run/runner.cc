#include "ion/context.h"
#include <GLFW/glfw3.h>
#include "ion/development/package.h"
#include <fstream>
#include <sstream>
#include "ion/assets.h"
#include "ion/physics.h"
#include "ion/render.h"
#include "ion/script.h"
#include "ion/defaults.h"
#include "ion/texture.h"
#include "ion/shader.h"
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

std::map<int, std::filesystem::path> ReadWorldList() {
	std::map<int, std::filesystem::path> worlds{};
	std::ifstream file("world_list.cfg");
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open world list file: world_list.cfg");
	}
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream line_stream(line);
		std::string world_id_str, path_str, load_type_str;
		if (std::getline(line_stream, world_id_str, ',') &&
			std::getline(line_stream, path_str, ',')) {
			int world_id = std::stoi(world_id_str);
			auto path = std::filesystem::path(path_str);
			worlds.insert({ world_id, path });
		}
	}
	return worlds;
}

int main() {
  if (!ion::GetSystem<AssetSystem>().CheckApplicationStructure()) {
    printf("Invalid application structure. Exiting.\n");
    return -1;
  }
	// Read file in world_list.cfg
	auto world_list = ReadWorldList();
	printf("Loaded world list with %zu worlds.\n", world_list.size());
  if (world_list.empty()) {
    printf("No worlds to load. Exiting.\n");
    return 0;
	}
	auto world_begin_path = world_list.begin()->second.string();
	auto world = ion::GetSystem<AssetSystem>().LoadAsset<World>(world_begin_path);
	printf("Loaded world from path: %s\n", world_begin_path.c_str());
	ion::GetSystem<RenderSystem>().Init();
	ion::GetSystem<PhysicsSystem>().Init();
	ion::GetSystem<ScriptSystem>().Init();

  AttributePointer position_pointer{
    .size = 3,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void*)0
  };
  AttributePointer texture_pointer{
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 5 * sizeof(float),
    .pointer = (void*)(3 * sizeof(float))
  };
  DataDescriptor data_desc{
    .pointers = {position_pointer, texture_pointer},
    .element_enabled = true,
    .vertices = vertices,
    .indices = indices
  };

  AttributePointer screen_position_pointer{
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void*)0
  };
  AttributePointer screen_texture_pointer{
    .size = 2,
    .type = DataType::FLOAT,
    .normalized = false,
    .stride = 4 * sizeof(float),
    .pointer = (void*)(2 * sizeof(float))
  };
  DataDescriptor screen_data_desc{
    .pointers = {screen_position_pointer, screen_texture_pointer},
    .element_enabled = true,
    .vertices = screen_vertices,
    .indices = indices
  };
  // save it first
	ion::GetSystem<AssetSystem>().SaveAsset<GPUData>("assets/screen_quad", std::make_shared<GPUData>(screen_data_desc, "assets/screen_quad"));
  //auto screen_data = ion::GetSystem<RenderSystem>().CreateData(screen_data_desc);
	auto screen_data = ion::GetSystem<AssetSystem>().LoadAsset<GPUData>("assets/screen_quad");

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

	ion::GetSystem<AssetSystem>().SaveAsset<GPUData>("assets/default_quad", std::make_shared<GPUData>(data_desc, "assets/default_quad"));
  auto defaults = Defaults{
    .default_color = ion::GetSystem<AssetSystem>().LoadAsset<Texture>("assets/test_sprite/color.png"),
		.default_normal = ion::GetSystem<AssetSystem>().LoadAsset<Texture>("assets/test_sprite/normal.png"),
    .default_shader = ion::GetSystem<AssetSystem>().LoadAsset<Shader>("assets/texture_shader"),
		.default_data = ion::GetSystem<AssetSystem>().LoadAsset<GPUData>("assets/default_quad")
  };

  while (!glfwWindowShouldClose(ion::GetSystem<RenderSystem>().GetWindow())) {
    glfwPollEvents();
    ion::GetSystem<ScriptSystem>().Update(world);
    ion::GetSystem<PhysicsSystem>().Update();
    game_system.Update(world); 

    ion::GetSystem<RenderSystem>().BindFramebuffer(color_buffer);
    ion::GetSystem<RenderSystem>().Clear();
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_COLOR);
    ion::GetSystem<RenderSystem>().BindFramebuffer(normal_buffer);
    ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
    ion::GetSystem<RenderSystem>().DrawWorld(world, RENDER_PASS_NORMAL);

    ion::GetSystem<RenderSystem>().BindFramebuffer(shaded);
    ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
    ion::GetSystem<RenderSystem>().Render(color_buffer, normal_buffer, screen_data, deferred_shader, world);
    ion::GetSystem<RenderSystem>().UnbindFramebuffer();

    // Bloom
    if (bloom_enable) {
      ion::GetSystem<RenderSystem>().BindFramebuffer(bloom_buffer);
      ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
      ion::GetSystem<RenderSystem>().UseShader(bloom_shader);
      ion::GetSystem<RenderSystem>().RunPass(shaded, bloom_buffer, bloom_shader, screen_data);

      ion::GetSystem<RenderSystem>().UseShader(bloom_blur_shader);
      bool horizontal = true;
      int blur_amount = 10;

      for (int i = 0; i < blur_amount; i++) {
        auto& source = horizontal ? bloom_buffer : bloom_buffer2;
        auto& target = horizontal ? bloom_buffer2 : bloom_buffer;
        ion::GetSystem<RenderSystem>().BindFramebuffer(target);
        ion::GetSystem<RenderSystem>().Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
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
  return 0;
	return 0;
}