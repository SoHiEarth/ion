#include "ion/script.h"
#include <filesystem>

bool CheckPythonExists() {
	// If the "Lib" directory doesn't exist, Python is not properly set up
	// To fix this, set the PYTHONHOME environment variable to point to your Python installation,
	// or copy the "Lib" directory from your Python installation to the directory the working directory.
	if (!std::filesystem::exists("Lib")) {
		return false;
	}
	return true;
}

void ScriptSystem::RunScript(std::string_view script,
	std::string_view func,
	std::map<std::string, std::string> parameters) {
	if (!python_initialized) {
		return;
	}
	auto pName = PyUnicode_DecodeFSDefault(script.data());
	auto pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (!pModule) {
		PyErr_Print();
		return;
	}
	auto pFunc = PyObject_GetAttrString(pModule, func.data());
	if (pFunc && PyCallable_Check(pFunc)) {
		PyObject* pDict = PyDict_New();
		for (const auto& [key, value] : parameters) {
			PyObject* pValue = PyUnicode_FromString(value.c_str());
			PyDict_SetItemString(pDict, key.c_str(), pValue);
			Py_DECREF(pValue);
		}
		PyObject* pArgs = PyTuple_Pack(1, pDict);
		Py_DECREF(pDict);
		PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
		Py_DECREF(pArgs);
		if (pValue != nullptr) {
			Py_DECREF(pValue);
		}
		else {
			PyErr_Print();
		}
	}
	else {
		if (PyErr_Occurred())
			PyErr_Print();
	}
	Py_XDECREF(pFunc);
	Py_DECREF(pModule);
}


void ScriptSystem::Init() {
	python_initialized = CheckPythonExists();
	if (!python_initialized) {
		printf("Python library not found, skipping initialization.\n");
		return;
	}
	Py_Initialize();
}

void ScriptSystem::Quit() {
	if (!python_initialized) {
		return;
	}
	Py_Finalize();
}

void ScriptSystem::Update(std::shared_ptr<World> world) {
	for (auto& [entity, script] : world->GetComponentSet<Script>()) {
		RunScript(script->path, script->module_name, script->parameters);
	}
}