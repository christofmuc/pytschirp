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
		def_->setInPatch(*patch_, value);
	}

	void set(py::list newlist) {
		std::vector<int> data;
		for (size_t i = 0; i < newlist.size(); i++) {
			if (py::int_(newlist[i]).check()) {
				data.push_back(py::int_(newlist[i]));
			}
			else {
				throw std::runtime_error("PyTschirp: Only lists of integers can be used as parameters!");
			}
		}
		def_->setInPatch(*patch_, data);
	}

	void setV(bool newValue) {
		def_->setInPatch(*patch_, newValue ? 1 : 0);
	}

	void setV(int newValue) {
		
	}

	void setV(int newValue, int relIndex) {
		if (def_->sysexIndex() + relIndex > def_->endSysexIndex()) {
			throw std::runtime_error("PyTschirp: Index out of range");
		}
		patch_->setAt(def_->sysexIndex() + relIndex, newValue);
	}

	py::object get() const {
		if (def_->type() == midikraft::SynthParameterDefinition::ParamType::INT_ARRAY) {
			std::vector<int> value;
			if (!def_->valueInPatch(*patch_, value)) {
				throw std::runtime_error("PyTschirp: Internal error getting array from patch data!");
			}
			py::list result;
			for (auto val : value) result.append(val);
			return result;
		}
		else {
			int value;
			if (def_->valueInPatch(*patch_, value)) {
				return py::int_(value);
			}
		}
		// Invalid
		throw std::runtime_error("PyTschirp: Invalid attribute index in patch");
	}

	std::string asText() const {
		return "not implemented"; // def_->valueAsText(get());
	}

	// Bindings not for python
	std::shared_ptr <midikraft::SynthParameterDefinition> def() {
		return def_;
	}

private:
	std::shared_ptr<PATCH> patch_;
	std::shared_ptr <midikraft::SynthParameterDefinition> def_;
};

