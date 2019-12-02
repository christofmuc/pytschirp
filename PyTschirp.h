#pragma once

#include "SynthParameterDefinition.h"
#include "Patch.h"

template<typename PATCH, typename SYNTHPARAM, typename PARAM_ID>
class PyTschirpAttribute {
public:
	PyTschirpAttribute(PATCH &patch, PARAM_ID const &param) : patch_(patch), def_(PATCH::find(param)) {
	}

	void set(bool newValue) {
		patch_.setAt(def_.sysexIndex(), newValue ? 1 : 0);
	}

	void set(int newValue) {
		patch_.setAt(def_.sysexIndex(), newValue);
	}

	int get() const {
		return patch_.value(def_);
	}

	std::string asText() const {
		return def_.valueAsText(get());
	}

private:
	PATCH &patch_;
	const midikraft::SynthParameterDefinition &def_;
};

template<typename PATCH, typename ATTRIBUTE, typename PARAM_ID>
class PyTschirp {
public:
	PyTschirp() {
	}

	ATTRIBUTE attr(PARAM_ID const &param) {
		return ATTRIBUTE(patch, param);
	}

private:
	PATCH patch;
};


