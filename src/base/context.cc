#include "context.h"
#include "assets.h"
#include "physics.h"
#include "render.h"
#include "script.h"

static AssetSystem asset_sys;
static RenderSystem render_sys;
static ScriptSystem script_sys;
static PhysicsSystem physics_sys;

template <> AssetSystem& ion::GetSystem<AssetSystem>() {
	return asset_sys;
}

template <> RenderSystem& ion::GetSystem<RenderSystem>() {
	return render_sys;
}

template <> PhysicsSystem& ion::GetSystem<PhysicsSystem>() {
	return physics_sys;
}

template <> ScriptSystem& ion::GetSystem<ScriptSystem>() {
	return script_sys;
}