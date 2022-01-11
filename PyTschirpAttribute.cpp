/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/


#include "PyTschirpAttribute.h"

#include "Capability.h"

#include "SynthParameterDefinition.h"
#include "DetailedParametersCapability.h"

namespace py = pybind11;

PyTschirpAttribute::PyTschirpAttribute(std::shared_ptr<midikraft::DataFile> patch, std::string const &param) : patch_(patch)
{
	def_ = defByName(param);
}

PyTschirpAttribute::PyTschirpAttribute(std::shared_ptr<midikraft::DataFile> patch, std::string const &param, int targetLayerNo) : patch_(patch)
{
	def_ = defByName(param);
	auto layerAccess = midikraft::Capability::hasCapability<midikraft::SynthMultiLayerParameterCapability>(def_);
	if (!layerAccess) {
		throw std::runtime_error("PyTschirp: Program Error: Parameter set does not support multi layers");
	}
	// This parameter definition targets a specific layer. This is used e.g. to access either Layer A or Layer B of a Prophet Rev2
	layerAccess->setSourceLayer(targetLayerNo);
	layerAccess->setTargetLayer(targetLayerNo);
}

void PyTschirpAttribute::set(int value)
{
	auto intParam = midikraft::Capability::hasCapability<midikraft::SynthIntParameterCapability>(def_);
	if (intParam) {
		intParam->setInPatch(*patch_, value);
	}
	else {
		throw std::runtime_error("PyTschirp: Illegal operation, can't set int type");
	}
}

void PyTschirpAttribute::set(std::vector<int> data)
{
	auto vectorParam = midikraft::Capability::hasCapability<midikraft::SynthVectorParameterCapability>(def_);
	if (vectorParam) {
		vectorParam->setInPatch(*patch_, data);
	}
	else {
		throw std::runtime_error("PyTschirp: Illegal operation, can't set vector type");
	}
}

py::object PyTschirpAttribute::get() const
{
	if (!def_) {
		return py::none();
	}
	if ((def_->type() == midikraft::SynthParameterDefinition::ParamType::INT_ARRAY)
		|| (def_->type() == midikraft::SynthParameterDefinition::ParamType::LOOKUP_ARRAY))
	{
		auto vectorParam = midikraft::Capability::hasCapability<midikraft::SynthVectorParameterCapability>(def_);
		if (vectorParam) {
			std::vector<int> value;
			if (!vectorParam->valueInPatch(*patch_, value)) {
				throw std::runtime_error("PyTschirp: Internal error getting array from patch data!");
			}
			py::list result;
			for (auto val : value) result.append(val);
			return result;
		}
		else {
			throw std::runtime_error("PyTschirp: Invalid type int array but no SynthVectorParameterCapability implemented");
		}
	}
	else {
		auto intParam = midikraft::Capability::hasCapability<midikraft::SynthIntParameterCapability>(def_);
		if (intParam) {
			int value;
			if (intParam->valueInPatch(*patch_, value)) {
				return py::int_(value);
			}
		}
		else {
			throw std::runtime_error("PyTschirp: Invalid type int but no SynthIntParameterCapability implemented");
		}
	}
	// Invalid
	throw std::runtime_error("PyTschirp: Invalid attribute index in patch");
}

std::string PyTschirpAttribute::asText() const
{
	if (def_) {
		return def_->valueInPatchToText(*patch_);
	}
	else {
		return "unknown attribute";
	}
}

std::shared_ptr <midikraft::SynthParameterDefinition> PyTschirpAttribute::def()
{
	return def_;
}

std::shared_ptr<midikraft::SynthParameterDefinition> PyTschirpAttribute::defByName(std::string const &name) const
{
	auto params = midikraft::Capability::hasCapability<midikraft::DetailedParametersCapability>(patch_);
	if (params) {
		for (auto param : params->allParameterDefinitions()) {
			if (param->name() == name) {
				return param;
			}
		}
	}
	return {};
}
