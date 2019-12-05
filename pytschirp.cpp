#include "PyTschirp.h"

#include "Logger.h"

#include "Rev2.h"
#include "Rev2Patch.h"

#include <pybind11/pybind11.h>

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

typedef PyTschirp<midikraft::Rev2Patch, PyAttribute_Rev2, std::string> PyTschirp_Rev2;

typedef PyTschirpSynth<midikraft::Rev2, PyTschirp_Rev2> PyTschirpSynth_Rev2;

PYBIND11_MODULE(pytschirp, m) {
	m.doc() = "Provide PyTschirp bindings for the Sequential Prophet Rev2";

	py::class_<PyTschirp_Rev2> rev2_tschirp(m, "Rev2Patch");
	rev2_tschirp.def(py::init<>())
		.def("attr", &PyTschirp_Rev2::attr)
		.def("__getattr__", &PyTschirp_Rev2::get_attr)
		.def("__setattr__", &PyTschirp_Rev2::set_attr);
				
	py::class_<PyAttribute_Rev2> rev2_attribute(m, "Rev2Attribute");
	rev2_attribute
		.def("set", py::overload_cast<int>(&PyAttribute_Rev2::set))
		.def("set", py::overload_cast<py::list>(&PyAttribute_Rev2::set))
		.def("get", &PyAttribute_Rev2::get)
		.def("asText", &PyAttribute_Rev2::asText)
		.def("__repr__", &PyAttribute_Rev2::asText)
		;
	py::class_<PyTschirpInvalidAttribute> pyTschirpInvalidAttribute(m, "InvalidAttribute");

	// Ints as text

	py::class_<PyTschirpSynth_Rev2> pyTschirpSynth(m, "Rev2");
	pyTschirpSynth
		.def(py::init<>())
		.def("detect", &PyTschirpSynth_Rev2::detect)
		.def("detected", &PyTschirpSynth_Rev2::detected)
		.def("location", &PyTschirpSynth_Rev2::location)
		.def("editBuffer", &PyTschirpSynth_Rev2::editBuffer);

	// TODO
	// sendPatchToEditBuffer
	// sendPatchToStoragePlace
	// savePatchAsSysex
	// loadSysex (Patch or vector of Patches)
	// make Patch "live" - all changes are sent as NRPNs directly to the synth

	// Fire up Singletons used by the frameworks we need
	new PythonLogger();

	// For use in PyTschirp, we need to lazily create the MidiController Singleton so it is in the right heap
	if (!midikraft::MidiController::instance()) {
		new midikraft::MidiController();
	}
	// And JUCE itself might not be fired up, so let's do that!
	juce::MessageManager::getInstance();
}


