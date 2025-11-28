#include "context.h"
#include "assets.h"
#include "physics.h"
#include "render.h"
#include "script.h"

RenderSystem Context::render_sys{};
AssetSystem Context::asset_sys{};
PhysicsSystem Context::physics_sys{};
ScriptSystem Context::script_sys{};