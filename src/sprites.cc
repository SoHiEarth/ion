#include "sprites.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "transform.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

void Sprites::New(Transform transform, Texture texture) {
  assert(transforms.size() == textures.size());
  transforms.push_back(transform);
  textures.push_back(texture);
}

void Sprites::Draw(Shader &shader, GetModelFlags model_flags) {
  assert(transforms.size() == textures.size());
  shader.Use();
  for (int i = 0; i < transforms.size(); i++) {
    RenderSystem::RenderSprite(transforms[i], model_flags, textures[i], shader);
  }
}

void Sprites::Inspector() {
  assert(transforms.size() == textures.size());
  for (int i = 0; i < transforms.size(); i++) {
    ImGui::Begin(std::format("Transform##{}", i).c_str());
    ImGui::Checkbox("Enable Physics", &transforms[i].enable_physics);
    auto old_position = transforms[i].position;
    ImGui::DragFloat3("Position", glm::value_ptr(transforms[i].position), 0.1F);
    if (transforms[i].position != old_position) {
      b2Body_SetTransform(
          transforms[i].body_id,
          b2Vec2(transforms[i].position.x, transforms[i].position.y),
          b2Body_GetRotation(transforms[i].body_id));
      b2Body_SetAwake(transforms[i].body_id, true);
    }
    ImGui::DragFloat2("Scale", glm::value_ptr(transforms[i].scale), 0.1F);

    float rotation_deg = glm::degrees(transforms[i].rotation);
    if (ImGui::DragFloat("Rotation", &rotation_deg, 1.0f)) {
      transforms[i].rotation = glm::radians(rotation_deg);
      b2Body_SetTransform(
          transforms[i].body_id,
          b2Vec2(transforms[i].position.x, transforms[i].position.y),
          b2MakeRot(transforms[i].rotation));
      b2Body_SetAwake(transforms[i].body_id, true);
    }
    ImGui::Image(textures[i].texture, ImVec2(100, 100));
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
        IM_ASSERT(payload->DataSize == sizeof(Texture));
        auto dropped = *(Texture *)payload->Data;
        textures[i] = dropped;
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::SameLine();
    ImGui::Text("Texture");
    ImGui::End();
  }
}
