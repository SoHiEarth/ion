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

const std::vector<float> vertices = {
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

const std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
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
  if (!ion::res::CheckApplicationStructure()) {
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
	auto world = ion::res::LoadAsset<World>(world_begin_path);
	printf("Loaded world from path: %s\n", world_begin_path.c_str());
	ion::render::Init();
	ion::physics::Init();
	ion::script::Init();
	auto screen_data = ion::res::LoadAsset<GPUData>("assets/screen_quad");
  auto framebuffer_info = FramebufferInfo{
      .recreate_on_resize = true
  };
  framebuffer_info.name = "Color";
  auto color_buffer = ion::render::CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Normal";
  auto normal_buffer = ion::render::CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Shaded";
  auto shaded = ion::render::CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom";
  auto bloom_buffer = ion::render::CreateFramebuffer(framebuffer_info);
  framebuffer_info.name = "Bloom 2";
  auto bloom_buffer2 = ion::render::CreateFramebuffer(framebuffer_info);
  auto& final_framebuffer = shaded;

  auto deferred_shader = ion::res::LoadAsset<Shader>("assets/deferred_shader");
  auto screen_shader = ion::res::LoadAsset<Shader>("assets/screen_shader");
  auto bloom_shader = ion::res::LoadAsset<Shader>("assets/bloom_shader");
  auto bloom_blur_shader = ion::res::LoadAsset<Shader>("assets/bloom_blur_shader");
  auto combine_shader = ion::res::LoadAsset<Shader>("assets/bloom_combine_shader");
  auto defaults = Defaults{};

  while (!glfwWindowShouldClose(ion::render::GetWindow())) {
    glfwPollEvents();
    ion::script::Update(world);
    ion::physics::Update(world);
    ion::game::Update(world); 

    ion::render::BindFramebuffer(color_buffer);
    ion::render::Clear();
    ion::render::DrawWorld(world, RENDER_PASS_COLOR);
    ion::render::BindFramebuffer(normal_buffer);
    ion::render::Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
    ion::render::DrawWorld(world, RENDER_PASS_NORMAL);

    ion::render::BindFramebuffer(shaded);
    ion::render::Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
    ion::render::Render(color_buffer, normal_buffer, screen_data, deferred_shader, world);
    ion::render::UnbindFramebuffer();

    // Bloom
    if (bloom_enable) {
      ion::render::BindFramebuffer(bloom_buffer);
      ion::render::Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
      ion::render::UseShader(bloom_shader);
      ion::render::RunPass(shaded, bloom_buffer, bloom_shader, screen_data);

      ion::render::UseShader(bloom_blur_shader);
      bool horizontal = true;
      int blur_amount = 10;

      for (int i = 0; i < blur_amount; i++) {
        auto& source = horizontal ? bloom_buffer : bloom_buffer2;
        auto& target = horizontal ? bloom_buffer2 : bloom_buffer;
        ion::render::BindFramebuffer(target);
        ion::render::Clear({ 0.0f, 0.0f, 0.0f, 1.0f });
        bloom_blur_shader->SetUniform("horizontal", (int)horizontal);
        ion::render::RunPass(source, target, bloom_blur_shader, screen_data);
        horizontal = !horizontal;
      }

      ion::render::UseShader(combine_shader);
      ion::render::BindTexture(shaded, 1);
      combine_shader->SetUniform("ION_PASS_FRAMEBUFFER", 1);
      ion::render::RunPass(bloom_buffer, shaded, combine_shader, screen_data);
    }

    ion::render::DrawFramebuffer(final_framebuffer, screen_shader, screen_data);
    ion::render::Present();
  }
  ion::physics::Quit();
  ion::script::Quit();
  ion::render::Quit();
  return 0;
}