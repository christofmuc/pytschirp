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
		return ATTRIBUTE(patch_, attrName);
	}

	void set_attr(std::string const &name, int value) {
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

	void set_attr(std::string const &name, std::vector<int> const &value) {
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

	std::string getName() {
		if (layerNo_ == -1) {
			return patch_->patchName();
		}
		else {
			auto layeredPatch = std::dynamic_pointer_cast<midikraft::LayeredPatch>(patch_);
			if (layeredPatch) {
				return layeredPatch->layerName(layerNo_);
			}
			else {
				throw std::runtime_error("PyTschirp: Program error: This is not a layered patch, but should be.");
			}
		}
	}

	void setName(std::string const &newName) {
		patch_->setName(newName);
	}

	PyTschirp layer(int layerNo) {
		auto layeredPatch = std::dynamic_pointer_cast<midikraft::LayeredPatch>(patch_);
		if (!layeredPatch) {
			throw std::runtime_error("PyTschirp: This is not a layered patch, can't retrieve layer");
		}

		// Create a new Tschirp that is the same as this one, but stores a layer number and thus will reroute all calls to the layer selected
		return PyTschirp(patch_, synth_, layerNo);
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


	//! Use this at your own risk
	std::shared_ptr<PATCH> patchPtr() {
		return patch_;
	}

private:
	// Private constructor to create a layer accessing Tschirp
	PyTschirp(std::shared_ptr<midikraft::Patch> p, std::weak_ptr<midikraft::Synth> synth, int layerNo) : PyTschirp(p, synth) {
		layerNo_ = layerNo;

		auto layeredPatch = std::dynamic_pointer_cast<midikraft::LayeredPatch>(patch_);
		if (layeredPatch) {
			if (!(layerNo >= 0 && layerNo < layeredPatch->numberOfLayers())) {
				throw std::runtime_error("PyTschirp: Invalid layer number given to layerName()");
			}
		}
	}

	std::string underscoreToSpace(std::string const &input) {
		auto copy = input;
		std::replace(copy.begin(), copy.end(), '_', ' ');
		return copy;
	}

	std::shared_ptr<PATCH> patch_;
	std::weak_ptr<midikraft::Synth> synth_;
	int layerNo_ = -1; // -1 means no layer is selected, access the whole patch. Else, this is the layer number this Tschirp represents
};

