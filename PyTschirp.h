/*
   Copyright (c) 2019 Christof Ruch. All rights reserved.

   Dual licensed: Distributed under Affero GPL license by default, an MIT license is available for purchase
*/

#pragma once

#include <pybind11/pybind11.h>

#include "SynthParameterDefinition.h"
#include "Synth.h"
#include "Patch.h"
#include "AutoDetection.h"

#include <algorithm>

namespace py = pybind11;

//TODO - should this really be a template class, or should this work with polymorphism?
template<typename PATCH, typename ATTRIBUTE>
class PyTschirp {
public:
	PyTschirp() {
		patch_ = std::make_shared<PATCH>();
	}

	PyTschirp(std::shared_ptr<midikraft::Patch> p, std::weak_ptr<midikraft::Synth> synth) {
		// Downcast possible?
		auto correctPatch = std::dynamic_pointer_cast<PATCH>(p);
		if (!correctPatch) {
			throw std::runtime_error("PyTschirp: Can't downcast, wrong patch type! Program error");
		}
		patch_ = correctPatch;
		synth_ = synth;
	}

	ATTRIBUTE attr(std::string const &param) {
		return ATTRIBUTE(patch_, param);
	}

	ATTRIBUTE get_attr(std::string const &attrName) {
		try {
			return ATTRIBUTE(patch_, attrName);
		}
		catch (std::runtime_error &) {
			// That doesn't seem to exist... try with spaces instead of underscores in the name
			return ATTRIBUTE(patch_, underscoreToSpace(attrName));
		}
	}

	void set_attr(std::string const &name, int value) {
		try {
			auto attr = ATTRIBUTE(patch_, name);
			attr.set(value);
			if (!synth_.expired() && synth_.lock()->channel().isValid()) {
				auto liveEditing = std::dynamic_pointer_cast<midikraft::SynthParameterLiveEditCapability>(attr.def());
				if (liveEditing) {
					// The synth is hot... we don't know if this patch is currently selected, but let's send the nrpn or other value changing message anyway!
					auto messages = liveEditing->setValueMessages(*patch_, synth_.lock().get());
					midikraft::MidiController::instance()->getMidiOutput(synth_.lock()->midiOutput())->sendBlockOfMessagesNow(messages);
				}
			}
		}
		catch (std::runtime_error &) {
			auto attr = ATTRIBUTE(patch_, underscoreToSpace(name));
			attr.set(value);
		}
	}

	void set_attr(std::string const &name, std::vector<int> const &value) {
		try {
			auto attr = ATTRIBUTE(patch_, name);
			attr.set(value);
			
			if (!synth_.expired() && synth_.lock()->channel().isValid()) {
				auto liveEditing = std::dynamic_pointer_cast<midikraft::SynthParameterLiveEditCapability>(attr.def());
				if (liveEditing) {
					// The synth is hot... we don't know if this patch is currently selected, but let's send the nrpn or other value changing message anyway!
					auto messages = liveEditing->setValueMessages(*patch_, synth_.lock().get());
					midikraft::MidiController::instance()->getMidiOutput(synth_.lock()->midiOutput())->sendBlockOfMessagesNow(messages);
				}
			}
		}
		catch (std::runtime_error &) {
			auto attr = ATTRIBUTE(patch_, underscoreToSpace(name));
			//attr.setV(value);
		}
	}

	// TODO - this looks like I don't actually need the Tschirp class? Could I move all the code here into the Patch class?
	// Idea: I could subclass the template PATCH class, then I don't need to reimplement stuff like this?
	std::string getName() {
		return patch_->patchName();
	}

	void setName(std::string const &newName) {
		patch_->setName(newName);
	}

	std::string layerName(int layerNo) {
		auto layeredPatch = std::dynamic_pointer_cast<midikraft::LayeredPatch>(patch_);
		if (layeredPatch) {
			if (layerNo >= 0 && layerNo < layeredPatch->numberOfLayers()) {
				return layeredPatch->layerName(layerNo);
			}
			throw std::runtime_error("PyTschirp: Invalid layer number given to layerName()");
		}
		if (layerNo == 0) {
			// Not a layered patch, but hey, layer 0 is good
			return patch_->patchName();
		}
		throw std::runtime_error("PyTschirp: This is not a layered patch, can't retrieve name of layer");
	}

	std::string toText() {
		return patch_->patchToTextRaw(false);
	}

	std::vector<std::string> parameterNames() {
		std::vector<std::string> result;
		for (auto p : patch_->allParameterDefinitions()) {
			result.push_back(p->name());
		}
		return result;
	}

private:
	std::string underscoreToSpace(std::string const &input) {
		auto copy = input;
		std::replace(copy.begin(), copy.end(), '_', ' ');
		return copy;
	}

	std::shared_ptr<PATCH> patch_;
	std::weak_ptr<midikraft::Synth> synth_;
};

