#pragma once
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include "ion/exports.h"
#include "ion/world.h"

namespace ion {
	namespace systems {
		enum class UpdatePhase {
			PRE_UPDATE,
			UPDATE,
			LATE_UPDATE
		};

		enum class UpdateCondition {
			ALWAYS,
			WHEN_PLAYING,
			WHEN_STOPPED
		};

		struct SystemInfo {
			std::string name;
			UpdatePhase phase;
			UpdateCondition condition;
			std::function<void(std::shared_ptr<World>&)> function;
			bool enabled = true;
		};

		namespace internal {
			extern std::vector<SystemInfo> systems;
			extern bool is_playing;
		}

		ION_API void RegisterSystem(const SystemInfo& info);
		ION_API void UpdateSystems(std::shared_ptr<World>& world, UpdatePhase phase);
		ION_API void SetSystemEnabled(const std::string& name, bool enabled);
		ION_API std::vector<SystemInfo>& GetSystems();
		ION_API bool GetState();
		ION_API void SetState(bool playing);
	}
}