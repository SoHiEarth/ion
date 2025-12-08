#include "ion/development/startwindow.h"
#include "ion/assets.h"
#include "ion/development/gui.h"
#include "ion/render.h"
#include <tinyfiledialogs/tinyfiledialogs.h>

std::shared_ptr<World> ion::gui::StartWindow() {
  ION_GUI_PREP_CONTEXT();
  auto func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
  char *path;
  while (!glfwWindowShouldClose(ion::render::GetWindow())) {
    glfwPollEvents();
    ion::render::Clear();
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
    ion::render::Present();
    switch (func) {
    case ion::gui::STARTWINDOW_LATEFUNC::NEW_WORLD_DIALOG:
      path = tinyfd_saveFileDialog("New World", nullptr, 0, nullptr, nullptr);
      if (path) {
        return ion::res::CreateAsset<World>(path);
      } else {
        func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
      }
      break;
    case ion::gui::STARTWINDOW_LATEFUNC::OPEN_WORLD_DIALOG:
      path = tinyfd_openFileDialog("Open World", nullptr, 0, nullptr, nullptr,
                                   false);
      if (path) {
        return ion::res::LoadAsset<World>(path, false);
      } else {
        func = ion::gui::STARTWINDOW_LATEFUNC::UNSPECIFIED;
      }
      break;
    default:
      break;
    }
  }
  throw std::runtime_error("Canceled Operation.");
}