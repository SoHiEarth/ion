#pragma once
#include <memory>
#include "ion/defaults.h"
void WorldInspector(std::shared_ptr<struct World>& world, Defaults& defaults);
void AssetInspector(std::shared_ptr<struct World>& world);