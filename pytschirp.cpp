/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#include "Logger.h"

#include "Rev2.h"
#include "Rev2Patch.h"

#include "PyTschirpPatch.h"
#include "PyTschirpAttribute.h"
#include "PyTschirpSynth.h"

#include <pybind11/pybind11.h>
//#include <pybind11/embed.h>
#include <pybind11/stl.h> // For vector to list

namespace py = pybind11;

// We will need a PythonLogger to get any debug output
class PythonLogger : public SimpleLogger {
public:
	virtual void postMessage(const String& message) override {
		std::cout << message << std::endl;
	}

	virtual void postMessageOncePerRun(const String& message) override {
		// ignore the once per run here
		postMessage(message);
	}
};

midikraft::MidiController *correctMidiController() {
	return  midikraft::MidiController::instance();
}

template <typename T> 
class SynthInstance : public PyTschirpSynth {
public:
	SynthInstance() : PyTschirpSynth(std::make_shared<T>()) {
	}
};

PYBIND11_MODULE(pytschirp, m) {
	m.doc() = "Provide PyTschirp bindings for the Sequential Prophet Rev2";

	py::class_<midikraft::MidiController> midiController(m, "MidiController");
	midiController.def(py::init<>());
	m.def("midiControllerInstance", &correctMidiController, py::return_value_policy::reference);

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

	//TODO
	// set name of patch/layer
				
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

	// TODO
	// sendPatchToEditBuffer
	// sendPatchToStoragePlace
	// get specific patch at specific location from synth

	// Fire up Singletons used by the frameworks we need
	new PythonLogger();

	// For use in PyTschirp, we need to lazily create the MidiController Singleton so it is in the right heap
	if (!midikraft::MidiController::instance()) {
		// Also, by default install a MIDI logger on stderr so we can see what is being sent and received
		midikraft::MidiController::instance()->setMidiLogFunction([](MidiMessage const &message, String const &source, bool isOut) {
			ignoreUnused(source);
			py::print(isOut ? "O: " : "I: ", message.getDescription().toStdString());
		});
	}
	// And JUCE itself might not be fired up, so let's do that!
	juce::MessageManager::getInstance();
}


