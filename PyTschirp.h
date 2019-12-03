#pragma once

#include "SynthParameterDefinition.h"
#include "Patch.h"

template<typename PATCH, typename SYNTHPARAM, typename PARAM_ID>
class PyTschirpAttribute {
public:
	PyTschirpAttribute(std::shared_ptr<PATCH> patch, PARAM_ID const &param) : patch_(patch), def_(PATCH::find(param)) {
	}

	void set(bool newValue) {
		patch_->setAt(def_->sysexIndex(), newValue ? 1 : 0);
	}

	void set(int newValue) {
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

private:
	std::shared_ptr<PATCH> patch_;
	std::shared_ptr <midikraft::SynthParameterDefinition> def_;
};

template<typename PATCH, typename ATTRIBUTE, typename PARAM_ID>
class PyTschirp {
public:
	PyTschirp() {
		patch = std::make_shared<PATCH>();
	}

	ATTRIBUTE attr(PARAM_ID const &param) {
		return ATTRIBUTE(patch, param);
	}

private:
	std::shared_ptr<PATCH> patch;
};


