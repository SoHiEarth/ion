#pragma once
#include <string>
#include <map>
#include "component.h"

struct Player {
	Transform transform;
	PhysicsBody physics_body;
	Renderable renderable;
};