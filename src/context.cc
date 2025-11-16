#include "context.h"
#include "render.h"
#include "assets.h"
#include "physics.h"

RenderSystem Context::render_sys{};
AssetSystem Context::asset_sys{};
PhysicsSystem Context::physics_sys{};