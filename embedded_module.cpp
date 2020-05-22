/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#include "embedded_module.h"

#include "PyTschirpPatch.h"
#include "PyTschirpAttribute.h"
#include "PyTschirpSynth.h"

#include "Rev2.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h> // For vector to list

namespace py = pybind11;

template <typename T>
class SynthInstance : public PyTschirpSynth {
public:
	SynthInstance() : PyTschirpSynth(std::make_shared<T>()) {
	}
};

PYBIND11_EMBEDDED_MODULE(pytschirpee, m) {
	m.doc() = "Provide PyTschirp bindings for the KnobKraft Orm";

	py::class_<PyTschirp> rev2_tschirp(m, "Patch");
	rev2_tschirp.def(py::init<std::shared_ptr<midikraft::Patch>>())
		.def("attr", &PyTschirp::get_attr)
		.def("__getattr__", &PyTschirp::get_attr)
		.def("__setattr__", py::overload_cast<std::string const &, int>(&PyTschirp::set_attr))
		.def("__setattr__", py::overload_cast<std::string const &, std::vector<int> const &>(&PyTschirp::set_attr))
		.def("__setitem__", py::overload_cast<std::string const &, int>(&PyTschirp::set_attr))
		.def("__setitem__", py::overload_cast<std::string const &, std::vector<int> const &>(&PyTschirp::set_attr))
		.def("__getitem__", &PyTschirp::get_attr)
		.def_property("name", &PyTschirp::getName, &PyTschirp::setName)
		.def("layer", &PyTschirp::layer)
		.def("parameterNames", &PyTschirp::parameterNames);

	py::class_<PyTschirpAttribute> rev2_attribute(m, "Attribute");
	rev2_attribute
		.def("set", py::overload_cast<int>(&PyTschirpAttribute::set))
		.def("set", py::overload_cast<std::vector<int>>(&PyTschirpAttribute::set))
		.def("get", &PyTschirpAttribute::get)
		.def("asText", &PyTschirpAttribute::asText)
		.def("__repr__", &PyTschirpAttribute::asText)
		;

	py::class_<SynthInstance<midikraft::Rev2>> pyTschirpSynth(m, "Rev2");
	pyTschirpSynth
		.def(py::init<>())
		.def("detect", &PyTschirpSynth::detect)
		.def("detected", &PyTschirpSynth::detected)
		.def("location", &PyTschirpSynth::location)
		.def("editBuffer", &PyTschirpSynth::editBuffer)
		.def("loadSysex", &PyTschirpSynth::loadSysex)
		.def("saveSysex", &PyTschirpSynth::saveSysex)
		.def("saveEditBuffer", &PyTschirpSynth::saveEditBuffer)
		.def("getGlobalSettings", &PyTschirpSynth::getGlobalSettings);

}

void globalImportEmbeddedModules() {
	py::module::import("pytschirpee");
}

std::string findPyTschirpModuleForSynth(std::string const &synthName)
{
	midikraft::Rev2 rev2;
	if (synthName == rev2.getName()) {
		return "Rev2";
	}
	return "";
}
