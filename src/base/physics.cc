#include "ion/physics.h"
#include <box2d/box2d.h>

namespace ion::physics::internal {
ION_API b2WorldId world = b2WorldId{};
} // namespace ion::physics::internal

b2WorldId ion::physics::GetWorld() { return internal::world; }

void ion::physics::Init() {
  auto world_def = b2DefaultWorldDef();
  world_def.gravity = b2Vec2(0.0F, -1.0F);
  internal::world = b2CreateWorld(&world_def);
}

void ion::physics::Update(std::shared_ptr<World> &world) {
  // Before update. Sync transforms --> physics bodies.
  for (auto &[entity, physics_body] : world->GetComponentSet<PhysicsBody>()) {
    if (physics_body->enabled && b2Body_IsValid(physics_body->body_id)) {
      auto transform = world->GetComponent<Transform>(entity);
      if (transform) {
        b2Vec2 body_position = b2Body_GetPosition(physics_body->body_id);
        float body_rotation =
            b2Rot_GetAngle(b2Body_GetRotation(physics_body->body_id));
        if (transform->position.x != body_position.x ||
            transform->position.y != body_position.y ||
            transform->rotation != body_rotation) {
          // Update physics body to match transform
          b2Body_SetAwake(physics_body->body_id, true);
          b2Body_SetTransform(
              physics_body->body_id,
              b2Vec2(transform->position.x, transform->position.y),
              b2Body_GetRotation(physics_body->body_id));
          // Set speed to zero to prevent motion after transform change
          b2Body_SetLinearVelocity(physics_body->body_id, b2Vec2(0.0, 0.0));
        }
      }
    }
  }
  b2World_Step(internal::world, 1.0f / 60.0f, 4);
  // After update. Sync physics bodies --> transforms.
  for (auto &[entity, physics_body] : world->GetComponentSet<PhysicsBody>()) {
    if (physics_body->enabled && b2Body_IsValid(physics_body->body_id)) {
      auto transform = world->GetComponent<Transform>(entity);
      if (transform) {
        b2Vec2 position = b2Body_GetPosition(physics_body->body_id);
        transform->position = glm::vec2(position.x, position.y);
        transform->rotation =
            b2Rot_GetAngle(b2Body_GetRotation(physics_body->body_id));
        printf("Entity Physics Update: ID %u, Position (%.2f, %.2f), Rotation "
               "%.2f\n",
               entity, transform->position.x, transform->position.y,
               transform->rotation);
      }
    }
    if (physics_body->enabled && !b2Body_IsAwake(physics_body->body_id)) {
      b2Body_SetAwake(physics_body->body_id, true);
    }
    if (physics_body->enabled == false &&
        b2Body_IsAwake(physics_body->body_id)) {
      b2Body_SetAwake(physics_body->body_id, false);
    }
  }
}

b2BodyId ion::physics::CreateBody(const std::shared_ptr<Transform> &transform) {
  b2BodyDef body_def = b2DefaultBodyDef();
  body_def.type = b2_dynamicBody;
  body_def.position = b2Vec2(transform->position.x, transform->position.y);
  b2BodyId body = b2CreateBody(internal::world, &body_def);
  b2Polygon shape =
      b2MakeBox(transform->scale.x * 0.5F, transform->scale.y * 0.5F);
  b2ShapeDef shape_def = b2DefaultShapeDef();
  shape_def.density = 1.0F;
  shape_def.material.friction = 0.3F;
  b2CreatePolygonShape(body, &shape_def, &shape);
  return body;
}

bool ion::physics::BodyIsValid(b2BodyId body) { return b2Body_IsValid(body); }

void ion::physics::Quit() {
  if (b2World_IsValid(internal::world)) {
    b2DestroyWorld(internal::world);
    internal::world = b2WorldId{};
  }
}
