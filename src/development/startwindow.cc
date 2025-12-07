#include "ion/development/startwindow.h"
#include "ion/development/gui.h"
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "ion/context.h"
#include "ion/render.h"
#include "ion/assets.h"

std::shared_ptr<World> ion::gui::StartWindow() {
  ION_GUI_PREP_CONTEXT();
  auto func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
  char* path;
  while (!glfwWindowShouldClose(ion::GetSystem<RenderSystem>().GetWindow())) {
    glfwPollEvents();
		ion::GetSystem<RenderSystem>().Clear();
    ion::gui::NewFrame();
    ImGui::Begin("Start", NULL, ImGuiWindowFlags_NoDecoration);
    if (ImGui::Button("New World")) {
      func = ion::gui::STARTWINDOW_LATEFUNC::NEW_WORLD_DIALOG;
    }
    if (ImGui::Button("Open World")) {
      func = ion::gui::STARTWINDOW_LATEFUNC::OPEN_WORLD_DIALOG;
    }
    ImGui::End();
    ion::gui::Render();
    ion::GetSystem<RenderSystem>().Present();
    switch (func) {
    case ion::gui::STARTWINDOW_LATEFUNC::NEW_WORLD_DIALOG:
      path = tinyfd_saveFileDialog("New World", nullptr, 0, nullptr, nullptr);
      if (path) {
        return ion::GetSystem<AssetSystem>().CreateAsset<World>(path);
      }
      else {
        func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
      }
      break;
    case ion::gui::STARTWINDOW_LATEFUNC::OPEN_WORLD_DIALOG:
      path = tinyfd_openFileDialog("Open World", nullptr, 0, nullptr, nullptr, false);
      if (path) {
        return ion::GetSystem<AssetSystem>().LoadAsset<World>(path, false);
      }
      else {
        func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
      }
      break;
    default:
      break;
    }
  }
  throw ion::gui::STARTWINDOW_EXCEPTION::CANCELLED;
}