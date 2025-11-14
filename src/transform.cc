#include "transform.h"
#include <box2d/box2d.h>
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
  position = {physics_position.x, physics_position.y, position.z};
  rotation = b2Rot_GetAngle(b2Body_GetRotation(body_id));
}

glm::mat4 Transform::GetModel(GetModelFlags flags) const {
  auto model = glm::mat4(1.0f);
  if (flags == GetModelFlags::IGNORE_Z) {
    model = glm::translate(model, glm::vec3(position.x, position.y, 0));
  } else {
    model = glm::translate(model, position);
  }
  model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(scale.x, scale.y, 1));
  return model;
}
