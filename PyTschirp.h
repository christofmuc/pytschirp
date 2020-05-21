/*
   Copyright (c) 2020 Christof Ruch. All rights reserved.

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

#include "PyTschirpAttribute.h"

//TODO - should this really be a template class, or should this work with polymorphism?
class PyTschirp {	
public:
	PyTschirp(std::shared_ptr<midikraft::Patch> patch) {
		patch_ = patch;
	}

	PyTschirp(std::shared_ptr<midikraft::DataFile> p, std::weak_ptr<midikraft::Synth> synth) {
		// Downcast possible?
		auto correctPatch = std::dynamic_pointer_cast<midikraft::Patch>(p);
		if (!correctPatch) {
			throw std::runtime_error("PyTschirp: Program error: Can't downcast, wrong patch type!");
		}
		patch_ = correctPatch;
		synth_ = synth;
	}

	PyTschirpAttribute get_attr(std::string const &attrName) {
		if (layerNo_ == -1) {
			return PyTschirpAttribute(patch_, attrName);
		}
		else {
			return PyTschirpAttribute(patch_, attrName, layerNo_);
		}
	}

	void set_attr(std::string const &name, int value) {
		auto attr = PyTschirpAttribute(patch_, name);
		attr.set(value);
		if (isChannelValid()) {
			auto liveEditing = std::dynamic_pointer_cast<midikraft::SynthParameterLiveEditCapability>(attr.def());
			if (liveEditing) {
				// The synth is hot... we don't know if this patch is currently selected, but let's send the nrpn or other value changing message anyway!
				auto messages = liveEditing->setValueMessages(*patch_, synth_.lock().get());
				midikraft::MidiController::instance()->getMidiOutput(midiOutput())->sendBlockOfMessagesNow(messages);
			}
		}
	}

	void set_attr(std::string const &name, std::vector<int> const &value) {
		auto attr = PyTschirpAttribute(patch_, name);
		attr.set(value);

		if (isChannelValid()) {
			auto liveEditing = std::dynamic_pointer_cast<midikraft::SynthParameterLiveEditCapability>(attr.def());
			if (liveEditing) {
				// The synth is hot... we don't know if this patch is currently selected, but let's send the nrpn or other value changing message anyway!
				auto messages = liveEditing->setValueMessages(*patch_, synth_.lock().get());
				midikraft::MidiController::instance()->getMidiOutput(midiOutput())->sendBlockOfMessagesNow(messages);
			}
		}
	}

	std::string getName() {
		if (layerNo_ == -1) {
			return patch_->name();
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
		auto storedName = std::dynamic_pointer_cast<midikraft::StoredPatchNameCapability>(patch_);
		if (patch_) {
			storedName->setName(newName);
		}
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
		auto params = std::dynamic_pointer_cast<midikraft::DetailedParametersCapability>(patch_);
		if (params) {
			for (auto p : params->allParameterDefinitions()) {
				result.push_back(p->name());
			}
		}
		return result;
	}

	//! Use this at your own risk
	std::shared_ptr<midikraft::Patch> patchPtr() {
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

	std::string midiInput() {
		if (!synth_.expired()) {
			auto location = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_.lock());
			if (location) {
				return location->midiInput();
			}
		}
		return "invalid";
	}

	std::string midiOutput() {
		if (!synth_.expired()) {
			auto location = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_.lock());
			if (location) {
				return location->midiOutput();
			}
		}
		return "invalid";
	}

	bool isChannelValid() const {
		if (!synth_.expired()) {
			auto location = std::dynamic_pointer_cast<midikraft::MidiLocationCapability>(synth_.lock());
			if (location) {
				return location->channel().isValid();
			}
		}
		return false;
	}

	std::shared_ptr<midikraft::Patch> patch_;
	std::weak_ptr<midikraft::Synth> synth_;
	int layerNo_ = -1; // -1 means no layer is selected, access the whole patch. Else, this is the layer number this Tschirp represents
};

