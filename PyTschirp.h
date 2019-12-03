#pragma once

#include "SynthParameterDefinition.h"
#include "Patch.h"

#include <algorithm>

class PyTschirpInvalidAttribute {
public:
	void set(int newValue) {
		ignoreUnused(newValue);
		throw new std::runtime_error("Can't set value of invalid attribute");
	}
};

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

	ATTRIBUTE get_attr(std::string const &attrName) {
		try {
			return ATTRIBUTE(patch, attrName);
		}
		catch (std::runtime_error &) {
			// That doesn't seem to exist... try with spaces instead of underscores in the name
			return ATTRIBUTE(patch, underscoreToSpace(attrName));
		}
	}

	void set_attr(std::string const &name, int value) {
		try {
			auto attr = ATTRIBUTE(patch, name);
			attr.set(value);
		}
		catch (std::runtime_error &) {
			auto attr = ATTRIBUTE(patch, underscoreToSpace(name));
			attr.set(value);
		}
	}

private:
	std::string underscoreToSpace(std::string const &input) {
		auto copy = input;
		std::replace(copy.begin(), copy.end(), '_', ' ');
		return copy;
	}

	std::shared_ptr<PATCH> patch;
};


