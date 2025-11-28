#include "script.h"

void ScriptSystem::RunScript(std::string_view script,
	std::string_view func,
	std::map<std::string, std::string> parameters) {
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
	Py_Initialize();
}

void ScriptSystem::Quit() {
	Py_Finalize();
}

void ScriptSystem::Update(std::shared_ptr<World> world) {
	for (auto& [entity, script] : world->GetComponentSet<Script>()) {
		RunScript(script.path, script.module_name, script.parameters);
	}
}