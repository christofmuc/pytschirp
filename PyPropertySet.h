/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

# pragma once

#include "JuceHeader.h"

#include "TypedNamedValue.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

class PyTypedNamedValue {
public:
	PyTypedNamedValue() = default;
	PyTypedNamedValue(std::shared_ptr<TypedNamedValue> value) : value_(value) {}

	py::object repr() const;
	py::object getValue() const;
	void setValue(py::handle const &o);

private:
	std::shared_ptr<TypedNamedValue> value_;
};


class PyPropertySet {
public:
	typedef std::map<std::string, PyTypedNamedValue> TPyPropertySet;
	using key_type = TPyPropertySet::key_type;

	PyPropertySet() = default;

	static TPyPropertySet buildFromVector(std::vector<std::shared_ptr<TypedNamedValue>> values);

	PyPropertySet(std::vector<std::shared_ptr<TypedNamedValue>> const &properties);

	py::iterator iter();
	PyTypedNamedValue getItem(PyPropertySet::key_type const &key);
	void setItem(PyPropertySet::key_type const &key, py::object const &o);

//private:
	TPyPropertySet propertySet_;
};



