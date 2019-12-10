/*
   Copyright (c) 2019 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include "JuceHeader.h"

class PyTschirpInvalidAttribute {
public:
	void set(int newValue) {
		ignoreUnused(newValue);
		throw new std::runtime_error("PyTschirp: Can't set value of invalid attribute");
	}
};

template<typename PATCH, typename SYNTHPARAM, typename PARAM_ID>
class PyTschirpAttribute {
public:
	PyTschirpAttribute(std::shared_ptr<PATCH> patch, PARAM_ID const &param) : patch_(patch), def_(PATCH::find(param)) {
	}

	void set(int value) {
		auto intParam = std::dynamic_pointer_cast<midikraft::SynthIntParameterCapability>(def_);
		if (intParam) {
			intParam->setInPatch(*patch_, value);
		}
		else {
			throw std::runtime_error("PyTschirp: Illegal operation, can't set int type");
		}
	}

	void set(std::vector<int> data) {
		auto vectorParam = std::dynamic_pointer_cast<midikraft::SynthVectorParameterCapability>(def_);
		if (vectorParam) {
			vectorParam->setInPatch(*patch_, data);
		}
		else {
			throw std::runtime_error("PyTschirp: Illegal operation, can't set vector type");
		}
	}

	py::object get() const {
		if (def_->type() == midikraft::SynthParameterDefinition::ParamType::INT_ARRAY) {
			auto vectorParam = std::dynamic_pointer_cast<midikraft::SynthVectorParameterCapability>(def_);
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
			auto intParam = std::dynamic_pointer_cast<midikraft::SynthIntParameterCapability>(def_);
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

	std::string asText() const {
		return def_->valueInPatchToText(*patch_);
	}

	// Bindings not for python
	std::shared_ptr <midikraft::SynthParameterDefinition> def() {
		return def_;
	}

private:
	std::shared_ptr<PATCH> patch_;
	std::shared_ptr <midikraft::SynthParameterDefinition> def_;
};

