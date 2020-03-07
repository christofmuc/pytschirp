/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#include "PyPropertySet.h"

py::object PyTypedNamedValue::repr() const
{
	return py::cast<py::str>(getValue());
}

py::object PyTypedNamedValue::getValue() const
{
	switch (value_->valueType) {
	case ValueType::Bool:
		return py::cast(bool(var(value_->value)));
	case ValueType::Integer:
		return py::cast(int(var(value_->value)));
	case ValueType::String:
		return py::cast(var(value_->value).operator juce::String().toStdString());
	case ValueType::Lookup:
		return py::cast(value_->lookup[(var)value_->value]);
	default:
		throw new std::runtime_error("pytschirp: Invalid property type given to pyobject cast");
	}
}

void PyTypedNamedValue::setValue(py::handle const &o)
{
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

PyPropertySet::TPyPropertySet PyPropertySet::buildFromVector(std::vector<std::shared_ptr<TypedNamedValue>> values)
{
	TPyPropertySet result;
	for (auto const &v : values) {
		result[v->name.toStdString()] = PyTypedNamedValue(v);
	}
	return result;
}

PyPropertySet::PyPropertySet(std::vector<std::shared_ptr<TypedNamedValue>> const &properties)
{
	propertySet_ = buildFromVector(properties);
}

py::iterator PyPropertySet::iter()
{
	return py::make_key_iterator(propertySet_.begin(), propertySet_.end());
}

PyTypedNamedValue PyPropertySet::getItem(key_type const &key)
{
	if (propertySet_.find(key) != propertySet_.end()) {
		return propertySet_[key];
	}
	throw new std::runtime_error("Key error"); //TODO how to throw the proper pybind error?
}

void PyPropertySet::setItem(key_type const &key, py::object const &o)
{
	if (propertySet_.find(key) != propertySet_.end()) {
		propertySet_[key].setValue(o);
		return;
	}
	throw new std::runtime_error("Key error"); //TODO how to throw the proper pybind error?
}
