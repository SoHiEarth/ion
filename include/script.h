#pragma once

#include <Python.h>
#include <map>
#include <string>
#include "world.h"

class ScriptSystem {
private:
	void RunScript(std::string_view script, std::string_view func = "main", std::map<std::string, std::string> parameters = {});
	bool python_initialized = false;
public:
	void Init();
	void Update(std::shared_ptr<World> world);
	void Quit();
};