#include "PyTschirp.h"

#include "Rev2.h"
#include "Rev2Patch.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

// Configure the template with the various classes for the Rev2
typedef PyTschirpAttribute<midikraft::Rev2Patch, midikraft::Rev2ParamDefinition, std::string> PyAttribute_Rev2;

typedef PyTschirp<midikraft::Rev2Patch, PyAttribute_Rev2, std::string> PyTschirp_Rev2;

PYBIND11_MODULE(matrix1000, m) {
	m.doc() = "Provide PyTschirp bindings for the Sequential Prophet Rev2";

	py::class_<PyTschirp_Rev2> rev2_tschirp(m, "Rev2Patch");
	rev2_tschirp.def(py::init<>())
		.def("attr", &PyTschirp_Rev2::attr);
				
	py::class_<PyAttribute_Rev2> rev2_attribute(m, "Rev2Attribute");
	rev2_attribute.def("set", py::overload_cast<bool>(&PyAttribute_Rev2::set))
		.def("set", py::overload_cast<int>(&PyAttribute_Rev2::set))
		.def("get", &PyAttribute_Rev2::get)
		.def("asText", &PyAttribute_Rev2::asText)
		;

	//py::class_<Matrix1000ParamDefinition> matrix1000ParamDefinition(m, "Matrix1000ParamDefinition");
}
