#pragma once
#include <memory>
#include "ion/exports.h"
#include "ion/defaults.h"

ION_API void WorldInspector(std::shared_ptr<struct World>& world, Defaults& defaults);
ION_API void AssetInspector();