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
	PyTypedNamedValue() {};

	PyTypedNamedValue(std::shared_ptr<TypedNamedValue> value) : value_(value) {}

	py::object getValue() const {
		switch (value_->valueType) {
		case ValueType::Bool:
			return py::cast(bool(var(value_->value)));
		case ValueType::Integer:
			return py::cast(int(var(value_->value)));
		case ValueType::String:
			return py::cast(var(value_->value).operator juce::String().toStdString());
		case ValueType::Lookup:
			return py::cast(value_->lookup[(var) value_->value]);
		default:
			throw new std::runtime_error("pytschirp: Invalid property type given to pyobject cast");
		}
	}

	void setValue(py::handle const &o) {
		switch (value_->valueType)
		{
		case ValueType::Bool:
			value_->value = o.cast<bool>();
			break;
		case ValueType::Integer:
			value_->value = o.cast<int>();
			break;
		case ValueType::String:
			value_->value = String(o.cast<std::string>());
			break;
		case ValueType::Lookup:
			value_->value = o.operator bool();
			break;
		default:
			throw new std::runtime_error("pytschirp: Invalid property type given to pyobject cast");
		}
	}

private:
	std::shared_ptr<TypedNamedValue> value_;
};

typedef std::map<std::string, PyTypedNamedValue> TPyPropertySet;

TPyPropertySet buildFromVector(std::vector<std::shared_ptr<TypedNamedValue>> values) {
	TPyPropertySet result;
	for (auto const &v : values) {
		result[v->name.toStdString()] = PyTypedNamedValue(v);
	}
	return result;
}


