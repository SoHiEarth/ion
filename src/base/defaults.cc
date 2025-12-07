#include "ion/defaults.h"
#include "ion/render.h"
#include "ion/assets.h"

Defaults::Defaults() {
  default_color = ion::res::LoadAsset<Texture>("assets/test_sprite/color.png", false),
    default_normal = ion::res::LoadAsset<Texture>("assets/test_sprite/normal.png", false),
    default_shader = ion::res::LoadAsset<Shader>("assets/texture_shader", false),
    default_data = ion::res::LoadAsset<GPUData>("assets/default_quad", false);
}