/*
   Copyright (c) 2019 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#include "PyTschirp.h"
#include "PyTschirpAttribute.h"
#include "PyTschirpSynth.h"

#include "Logger.h"

#include "Rev2.h"
#include "Rev2Patch.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For vector to list

namespace py = pybind11;

// We will need a PythonLogger to get any debug output
class PythonLogger : public SimpleLogger {
public:
	virtual void postMessage(const String& message) override {
		std::cout << message << std::endl;
	}
};

// Configure the template with the various classes for the Rev2
typedef PyTschirpAttribute<midikraft::Rev2Patch, midikraft::Rev2ParamDefinition, std::string> PyAttribute_Rev2;

typedef PyTschirp<midikraft::Rev2Patch, PyAttribute_Rev2> PyTschirp_Rev2;

typedef PyTschirpSynth<midikraft::Rev2, PyTschirp_Rev2> PyTschirpSynth_Rev2;

PYBIND11_MODULE(pytschirp, m) {
	m.doc() = "Provide PyTschirp bindings for the Sequential Prophet Rev2";

	py::class_<PyTschirp_Rev2> rev2_tschirp(m, "Rev2Patch");
	rev2_tschirp.def(py::init<>())
		.def("attr", &PyTschirp_Rev2::attr)
		.def("__getattr__", &PyTschirp_Rev2::get_attr)
		.def("__setattr__", py::overload_cast<std::string const &, int>(&PyTschirp_Rev2::set_attr))
		.def("__setattr__", py::overload_cast<std::string const &, std::vector<int> const &>(&PyTschirp_Rev2::set_attr))
		.def("__setitem__", py::overload_cast<std::string const &, int>(&PyTschirp_Rev2::set_attr))
		.def("__setitem__", py::overload_cast<std::string const &, std::vector<int> const &>(&PyTschirp_Rev2::set_attr))
		.def("__getitem__", &PyTschirp_Rev2::get_attr)
		.def_property("name", &PyTschirp_Rev2::getName, &PyTschirp_Rev2::setName)
		.def("layerName", &PyTschirp_Rev2::layerName)
		.def("toText", &PyTschirp_Rev2::toText)
		.def("parameterNames", &PyTschirp_Rev2::parameterNames);

	//TODO
	// set name of patch/layer
				
	py::class_<PyAttribute_Rev2> rev2_attribute(m, "Rev2Attribute");
	rev2_attribute
		.def("set", py::overload_cast<int>(&PyAttribute_Rev2::set))
		.def("set", py::overload_cast<std::vector<int>>(&PyAttribute_Rev2::set))
		.def("get", &PyAttribute_Rev2::get)
		.def("asText", &PyAttribute_Rev2::asText)
		.def("__repr__", &PyAttribute_Rev2::asText)
		;
	py::class_<PyTschirpInvalidAttribute> pyTschirpInvalidAttribute(m, "InvalidAttribute");

	py::class_<PyTschirpSynth_Rev2> pyTschirpSynth(m, "Rev2");
	pyTschirpSynth
		.def(py::init<>())
		.def("detect", &PyTschirpSynth_Rev2::detect)
		.def("detected", &PyTschirpSynth_Rev2::detected)
		.def("location", &PyTschirpSynth_Rev2::location)
		.def("editBuffer", &PyTschirpSynth_Rev2::editBuffer)
		.def("loadSysex", &PyTschirpSynth_Rev2::loadSysex)
		.def("saveSysex", &PyTschirpSynth_Rev2::saveSysex)
		.def("saveEditBuffer", &PyTschirpSynth_Rev2::saveSysex);

	// TODO
	// sendPatchToEditBuffer
	// sendPatchToStoragePlace
	// get specific patch at specific location from synth

	// Fire up Singletons used by the frameworks we need
	new PythonLogger();

	// For use in PyTschirp, we need to lazily create the MidiController Singleton so it is in the right heap
	if (!midikraft::MidiController::instance()) {
		new midikraft::MidiController();

		// Also, by default install a MIDI logger on stderr so we can see what is being sent and received
		midikraft::MidiController::instance()->setMidiLogFunction([](MidiMessage const &message, String const &source, bool isOut) {
			py::print(isOut ? "O: " : "I: ", message.getDescription().toStdString());
		});
	}
	// And JUCE itself might not be fired up, so let's do that!
	juce::MessageManager::getInstance();
}


