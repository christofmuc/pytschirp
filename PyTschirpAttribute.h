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
		// Depending on what type this object is, we might be successful or not

		if (py::int_(value).check()) {
			// This is an integer, use the integer setter
			int v = py::int_(value);
			setV(v);
		}
		else {
			throw std::runtime_error("PyTschirp: Type can not be set to parameter");
		}
	}

	void set(py::list newlist) {
		// Ups, this is a list. Only some synth parameters are lists (e.g. gated sequencer data), so we need some additional 
		// checks before we can set this...
		if (def_->sysexIndex() != def_->endSysexIndex()) {
			// This is indeed an array type of synth parameter
			if (py::len(newlist) != (def_->endSysexIndex() - def_->sysexIndex() + 1)) {
				throw std::runtime_error("PyTschirp: List length doesn't match parameter length");
			}
		}
		else {
			throw std::runtime_error("PyTschirp: Type can not be set to list");
		}
	}

	void setV(bool newValue) {
		patch_->setAt(def_->sysexIndex(), newValue ? 1 : 0);
	}

	void setV(int newValue) {
		patch_->setAt(def_->sysexIndex(), newValue);
	}

	int get() const {
		int value;
		if (def_->valueInPatch(*patch_, value)) {
			return value;
		}
		// Invalid
		throw new std::runtime_error("PyTschirp: Invalid attribute index in patch");
	}

	std::string asText() const {
		return def_->valueAsText(get());
	}

	// Bindings not for python
	std::shared_ptr <midikraft::SynthParameterDefinition> def() {
		return def_;
	}

private:
	std::shared_ptr<PATCH> patch_;
	std::shared_ptr <midikraft::SynthParameterDefinition> def_;
};

