#include "transform.h"
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
    b2Body_SetTransform(body_id, b2Vec2(position.x, position.y),
                        b2Body_GetRotation(body_id));
    b2Body_SetAwake(body_id, false);
    return;
  }

  b2Body_SetAwake(body_id, true);
  b2Vec2 physics_position = b2Body_GetPosition(body_id);
  position.x = physics_position.x;
  position.y = physics_position.y;
}

void Transform::RenderInspector(int id) {
  ImGui::Begin(std::format("Transform##{}", id).c_str());
  ImGui::Checkbox("Enable Physics", &enable_physics);
  ImGui::DragFloat2("Position", glm::value_ptr(position), 0.1F);
  ImGui::DragInt("Layer", &layer);
  ImGui::DragFloat2("Scale", glm::value_ptr(scale), 0.1F);
  ImGui::DragFloat("Rotation", &rotation, 0.1F);
  ImGui::End();
}

glm::mat4 Transform::GetModel() {
  auto model =
      glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, layer));
  model =
      glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(scale.x, scale.y, 1));
  return model;
}
