#pragma once
#include <box2d/box2d.h>

class PhysicsSystem {
private:
  b2WorldId world;

public:
  b2WorldId GetWorld();
  void Init();
  void Update();
  void Quit();
};
