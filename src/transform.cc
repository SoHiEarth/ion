#include "transform.h"
#include "texture.h"
#include <box2d/box2d.h>
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

Transform::Transform(b2WorldId world) {
  b2BodyDef body_def = b2DefaultBodyDef();
  body_def.type = b2_dynamicBody;
  body_def.position = b2Vec2(position.x, position.y);
  body_id = b2CreateBody(world, &body_def);
  b2Polygon shape = b2MakeBox(scale.x * 0.5F, scale.y * 0.5F);
  b2ShapeDef shape_def = b2DefaultShapeDef();
  shape_def.density = 1.0F;
  shape_def.material.friction = 0.3F;
  b2CreatePolygonShape(body_id, &shape_def, &shape);
}

void Transform::UpdatePhysics() {
  if (!enable_physics) {
    b2Body_SetType(body_id, b2_staticBody);
    return;
  }

  if (b2Body_GetType(body_id) == b2_staticBody) {
    b2Body_SetType(body_id, b2_dynamicBody);
  }

  auto physics_position = b2Body_GetPosition(body_id);
  position = {physics_position.x, physics_position.y};
  rotation = b2Rot_GetAngle(b2Body_GetRotation(body_id));
}

void Transform::RenderInspector(int id) {
  ImGui::Begin(std::format("Transform##{}", id).c_str());
  ImGui::Checkbox("Enable Physics", &enable_physics);
  glm::vec2 old_position = position;
  ImGui::DragFloat2("Position", glm::value_ptr(position), 0.1F);
  if (position != old_position) {
    b2Body_SetTransform(body_id, b2Vec2(position.x, position.y),
                        b2Body_GetRotation(body_id));
    b2Body_SetAwake(body_id, true);
  }
  ImGui::DragInt("Layer", &layer);
  ImGui::DragFloat2("Scale", glm::value_ptr(scale), 0.1F);

  float rotation_deg = glm::degrees(rotation);
  if (ImGui::DragFloat("Rotation", &rotation_deg, 1.0f)) {
    rotation = glm::radians(rotation_deg);
    b2Body_SetTransform(body_id, b2Vec2(position.x, position.y),
                        b2MakeRot(rotation));
    b2Body_SetAwake(body_id, true);
  }

  ImGui::Image(texture.texture, ImVec2(100, 100));
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {
      IM_ASSERT(payload->DataSize == sizeof(Texture));
      auto dropped = *(Texture *)payload->Data;
      texture = dropped;
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::SameLine();
  ImGui::Text("Texture");

  ImGui::End();
}

Texture &Transform::GetTexture() { return texture; }

glm::mat4 Transform::GetModel(GetModelFlags flags) {
  auto model = glm::mat4(1.0f);
  if (flags == GetModelFlags::IGNORE_LAYER) {
    model = glm::translate(model, glm::vec3(position.x, position.y, 0));
  } else {
    model = glm::translate(model, glm::vec3(position.x, position.y, layer));
  }
  model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(scale.x, scale.y, 1));
  return model;
}
