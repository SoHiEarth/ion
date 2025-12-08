#pragma once

#include "exports.h"
#include "world.h"
#include <map>
#include <memory>
#include <string>

namespace ion::script {
namespace internal {
void RunScript(std::string_view script, std::string_view func = "main",
               std::map<std::string, std::string> parameters = {});
ION_API extern bool python_initialized;
} // namespace internal

void Init();
void Update(std::shared_ptr<World> world);
void Quit();
} // namespace ion::script