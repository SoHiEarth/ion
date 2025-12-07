#pragma once
#include "exports.h"
#include <box2d/box2d.h>

namespace ion {
  namespace physics {
    namespace internal {
      ION_API extern b2WorldId world;
		} // namespace internal
  ION_API b2WorldId GetWorld();
  ION_API void Init();
  ION_API void Update();
  ION_API void Quit();
	} // namespace physics
} // namespace ion