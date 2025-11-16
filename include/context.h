#pragma once
class AssetSystem;
class RenderSystem;
class PhysicsSystem;

class Context {
  public:
    static AssetSystem asset_sys;
    static RenderSystem render_sys;
    static PhysicsSystem physics_sys;
    static Context& Get() {
      static Context instance;
      return instance;
    }
};